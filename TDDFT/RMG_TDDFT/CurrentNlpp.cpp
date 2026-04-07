/*
 *
 * Copyright 2014 The RMG Project Developers. See the COPYRIGHT file 
 * at the top-level directory of this distribution or in the current
 * directory.
 * 
 * This file is part of RMG. 
 * RMG is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * any later version.
 *
 * RMG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <complex>
#include "FiniteDiff.h"
#include "const.h"
#include "rmgtypedefs.h"
#include "typedefs.h"
#include "rmg_error.h"
#include "rmgthreads.h"
#include "RmgTimer.h"
#include "RmgThread.h"
#include "rmg_reduce.h"
#include "Kpoint.h"
#include "rmg_gemm.h"
#include "Subdiag.h"
#include "GpuAlloc.h"

#include "blas.h"
#include "blacs.h"
#include "RmgParallelFft.h"
#include "RmgException.h"


#include "common_prototypes.h"
#include "common_prototypes1.h"
#include "transition.h"
#include "prototypes_tddft.h"

/*
 * ============================================================================
 * CurrentNlpp — Nonlocal pseudopotential correction to the current operator
 * ============================================================================
 *
 * PURPOSE
 * -------
 * Adds the nonlocal pseudopotential (NL-PP) contribution to the momentum
 * matrix Pxmatrix/Pymatrix/Pzmatrix that was initialized by VecPHmatrix.
 *
 * THEORY
 * ------
 * With a nonlocal pseudopotential V_NL, the velocity (current) operator is
 * NOT simply p/m.  The full current operator for the Hamiltonian
 *
 *   H = p^2/2 + V_loc + V_NL
 *
 * is obtained from the Heisenberg equation of motion for the position:
 *
 *   v = dr/dt = i[H, r] = p + i[V_NL, r]
 *
 * The second term, i[V_NL, r], is the nonlocal correction.  For a
 * separable NCPP  V_NL = sum_I sum_lm D_lm |beta_lm><beta_lm|, this gives:
 *
 *   <phi_i | i[V_NL, r_alpha] | phi_j>
 *       = i * sum_I sum_lm D_lm [<phi_i|beta_lm> <beta_lm|r_alpha|phi_j>
 *                                - <phi_i|r_alpha|beta_lm> <beta_lm|phi_j>]
 *
 * In practice this is computed by applying the NL projectors projected
 * onto each Cartesian direction (AppNls_0xyz with direction flags 1,2,3
 * for x,y,z), then forming the matrix elements by GEMM against all orbitals.
 *
 * RESULT
 * ------
 * The += accumulation into Pxmatrix/Pymatrix/Pzmatrix means that after
 * CurrentNlpp, those matrices contain the TOTAL current operator:
 *
 *   P^alpha_ij  (total)  =  P^alpha_ij (kinetic, from VecPHmatrix)
 *                         + P^alpha_ij (nonlocal PP, from CurrentNlpp)
 *
 * These complete matrices are then used for:
 *   (a) J(t) = Re[Tr(P(t) * P^alpha)]   — current density at each step
 *   (b) The initial kick H(t=0) += A_0 * eps * P^alpha  (only kinetic part
 *       from VecPHmatrix is used there; CurrentNlpp affects only J, not H).
 * ============================================================================
 */
template void CurrentNlpp<std::complex<double>> (Kpoint<std::complex<double>> *kptr, int *desca, int tddft_start_state, int num_states);
template void CurrentNlpp<double> (Kpoint<double> *kptr, int *desca, int tddft_start_state, int num_states);
template <typename OrbitalType>
void CurrentNlpp (Kpoint<OrbitalType> *kptr, int *desca, int tddft_start_state, int num_states)
{

    int ictxt=desca[1], mb=desca[4], nb=desca[5], mxllda = desca[8];
    int mycol, myrow, nprow, npcol;
    Cblacs_gridinfo(ictxt, &nprow, &npcol, &myrow, &mycol);
    rmg::grid *G = kptr->G;
    Lattice *L = kptr->L;

    int pbasis = kptr->pbasis;
    int pbasis_noncol = pbasis * ct.noncoll_factor;

    double vel = L->get_omega() / ((double)(G->get_NX_GRID(1) * G->get_NY_GRID(1) * G->get_NZ_GRID(1)));
    //  alpha take care of i in moment operator
    OrbitalType alpha(vel);
    OrbitalType beta(0.0);
    std::complex<double> I_t(0.0, 1.0);

    OrbitalType *block_matrix;

    char *trans_n = "n";
    char *trans_c = "c";
    char *trans_a;
    trans_a = trans_c;

    //block_size = num_states;
    int num_blocks = (num_states + nb -1)/nb;
    // First time through allocate pinned memory for global_matrix1

    // 3 block matrix for px, py, pz operators
    int retval1 = MPI_Alloc_mem(3*num_states * nb * sizeof(OrbitalType) , MPI_INFO_NULL, &block_matrix);
    if(retval1 != MPI_SUCCESS) {
        rmg::error("Memory allocation failure ");
    }

    OrbitalType *block_matrix_x = block_matrix;
    OrbitalType *block_matrix_y = block_matrix_x + num_states * nb;
    OrbitalType *block_matrix_z = block_matrix_y + num_states * nb;


    // V|psi> is in tmp_arrayT
    OrbitalType *psi = kptr->orbital_storage + tddft_start_state * pbasis_noncol;
    OrbitalType *psi_dev;
    if(kptr->psi_dev)
    {
        psi_dev = kptr->psi_dev + tddft_start_state * pbasis_noncol;
    }
    else
    {
        psi_dev = psi;
    }

    OrbitalType *psi_x = &kptr->orbital_storage[kptr->nstates * pbasis_noncol];  // use the memory of psi extra 3* state_block_size.
    OrbitalType *psi_y = &kptr->orbital_storage[kptr->nstates * pbasis_noncol] + nb * pbasis_noncol;  // use the memory of psi extra 3* state_block_size.
    OrbitalType *psi_z = &kptr->orbital_storage[kptr->nstates * pbasis_noncol] + 2*nb * pbasis_noncol;  // use the memory of psi extra 3* state_block_size.

    OrbitalType *ns = kptr->ns;
    OrbitalType *nv = kptr->nv;
    OrbitalType *newsint_local = kptr->newsint_local;


    int factor = sizeof(OrbitalType)/sizeof(double);
    int ix, iy, iz;
    Rmg_G->pe2xyz (pct.gridpe, &ix, &iy, &iz);

    for(int ib = 0; ib < num_blocks; ib++)
    {
        // upper triagle blocks only
        // ib : (nstates - ib * nb) * block_size matrix
        // ib = 0: nstates * block_size matrix
        // ib = 1: (nstates - nb) * block_size matrix
        // block index (ib, ib:num_blocks)
        // this_block_size will be nb for the first num_blocks-1 block, the last block could be smaller than nb

        int this_block_size, length_block;
        length_block = num_states - ib * nb;
        this_block_size = std::min(nb, length_block);
        int st_start = ib *nb + tddft_start_state;

        // Apply NL projectors weighted by the Cartesian coordinate direction:
        //   AppNls_0xyz(..., direction=1) -> nv = sum_I D_lm <beta_lm|phi_j> * x-component
        //   direction=2 -> y-component,  direction=3 -> z-component
        // Result nv[r] represents the action of i[V_NL, r_alpha] on the orbital block.
        AppNls_0xyz(kptr, newsint_local, kptr->Kstates[0].psi, nv, ns, st_start, this_block_size, 1);
        for (int idx = 0; idx < this_block_size * pbasis_noncol; idx++)
        {
            psi_x[idx] = nv[idx]  ;
        }
        AppNls_0xyz(kptr, newsint_local, kptr->Kstates[0].psi, nv, ns, st_start, this_block_size, 2);
        for (int idx = 0; idx < this_block_size * pbasis_noncol; idx++)
        {
            psi_y[idx] = nv[idx]  ;
        }
        AppNls_0xyz(kptr, newsint_local, kptr->Kstates[0].psi, nv, ns, st_start, this_block_size, 3);
        for (int idx = 0; idx < this_block_size * pbasis_noncol; idx++)
        {
            psi_z[idx] = nv[idx]  ;
        }
        rmg::gemm(trans_a, trans_n, this_block_size, num_states,  pbasis_noncol, alpha, psi_x, pbasis_noncol, psi_dev,
                pbasis_noncol, beta, block_matrix_x, this_block_size);
        rmg::block_reduce((double *)block_matrix_x, (size_t)this_block_size * (size_t)num_states * (size_t)factor , pct.grid_comm);

        rmg::gemm(trans_a, trans_n, this_block_size, num_states,  pbasis_noncol, alpha, psi_y, pbasis_noncol, psi_dev,
                pbasis_noncol, beta, block_matrix_y, this_block_size);
        rmg::block_reduce((double *)block_matrix_y, (size_t)this_block_size * (size_t)num_states * (size_t)factor , pct.grid_comm);

        rmg::gemm(trans_a, trans_n, this_block_size, num_states,  pbasis_noncol, alpha, psi_z, pbasis_noncol, psi_dev,
                pbasis_noncol, beta, block_matrix_z, this_block_size);
        rmg::block_reduce((double *)block_matrix_z, (size_t)this_block_size * (size_t)num_states * (size_t)factor , pct.grid_comm);

        if(ct.tddft_tiledMM)
        {
            int istart = ib *nb;
            for(int i = 0; i < this_block_size; i++)
            {
                for(int j = 0; j < num_states/pct.local_comm_npes; j++)
                {
                    int jglob = j + pct.local_rank * num_states/pct.local_comm_npes ;
                    kptr->Pxmatrix_cpu[ j * num_states + i + istart] += I_t * block_matrix_x[ jglob * this_block_size + i];
                    kptr->Pymatrix_cpu[ j * num_states + i + istart] += I_t * block_matrix_y[ jglob * this_block_size + i];
                    kptr->Pzmatrix_cpu[ j * num_states + i + istart] += I_t * block_matrix_z[ jglob * this_block_size + i];
                }
            }

            for(int i = 0; i < this_block_size; i++)
            {
                int itile = i + istart - pct.local_rank * num_states/pct.local_comm_npes;
                if(itile >= 0 && itile < num_states/pct.local_comm_npes)
                {
                    for(int j = 0; j < num_states; j++)
                    {
                        kptr->Pxmatrix_cpu[ itile * num_states + j] += MyConj(I_t * block_matrix_x[ j * this_block_size + i]);
                        kptr->Pymatrix_cpu[ itile * num_states + j] += MyConj(I_t * block_matrix_y[ j * this_block_size + i]);
                        kptr->Pzmatrix_cpu[ itile * num_states + j] += MyConj(I_t * block_matrix_z[ j * this_block_size + i]);
                    }
                }
            }

        }

        else
        {
            //block_matrix to distHij;
            if(myrow == ib%nprow)
            {
                int istart = (ib/nprow) *nb;
                for(int jb = 0; jb < num_blocks; jb++)
                {
                    if(mycol == jb%npcol)
                        //  block (ib,jb) in this processor
                    {
                        int this_block_size_col  = std::min(mb, num_states - mb * jb);
                        int jstart = (jb/npcol) * mb;
                        for(int i = 0; i < this_block_size; i++)
                        {
                            for(int j = 0; j < this_block_size_col; j++)
                            {
                                kptr->Pxmatrix_cpu[(jstart + j) * mxllda + i + istart] += I_t * block_matrix_x[ (j + jb * mb ) * this_block_size + i];
                                kptr->Pymatrix_cpu[(jstart + j) * mxllda + i + istart] += I_t * block_matrix_y[ (j + jb * mb ) * this_block_size + i];
                                kptr->Pzmatrix_cpu[(jstart + j) * mxllda + i + istart] += I_t * block_matrix_z[ (j + jb * mb ) * this_block_size + i];
                            }
                        }
                    }
                }
            }

            if(mycol == ib%npcol)
            {
                int istart = (ib/npcol) *nb;
                for(int jb = 0; jb < num_blocks; jb++)
                {
                    if(myrow == jb%nprow)
                        //  block (jb,ib) in this processor
                    {
                        int this_block_size_col  = std::min(mb, num_states - mb * jb);
                        int jstart = (jb/nprow) * mb;
                        for(int i = 0; i < this_block_size; i++)
                        {
                            for(int j = 0; j < this_block_size_col; j++)
                            {
                                kptr->Pxmatrix_cpu[(istart + i) * mxllda + j + jstart] += MyConj(I_t * block_matrix_x[ (j + jb * mb ) * this_block_size + i]);
                                kptr->Pymatrix_cpu[(istart + i) * mxllda + j + jstart] += MyConj(I_t * block_matrix_y[ (j + jb * mb ) * this_block_size + i]);
                                kptr->Pzmatrix_cpu[(istart + i) * mxllda + j + jstart] += MyConj(I_t * block_matrix_z[ (j + jb * mb ) * this_block_size + i]);
                            }
                        }
                    }
                }
            }

        }
    }

    delete [] block_matrix;
    //  rmg::printlog("kvec %f", kptr->kp.kvec[0] );
    //  for(int i = 0; i < 8; i++)
    //  {
    //      rmg::printlog("\n aaa ");
    //      for(int j = 0; j < 8; j++)
    //          rmg::printlog(" %8.3e ", std::real(kptr->Pxmatrix_cpu[i *8 + j]));
    //  }
    //  for(int i = 0; i < 8; i++)
    //  {
    //      rmg::printlog("\n bbb ");
    //      for(int j = 0; j < 8; j++)
    //          rmg::printlog(" %8.3e ", std::imag(kptr->Pxmatrix_cpu[i *8 + j]));
    //  }

}
