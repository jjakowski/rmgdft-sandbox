/*
 *
 * Copyright 2026 The RMG Project Developers. See the COPYRIGHT file 
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

/*
 * ============================================================================
 * rmg::tddft — Real-Time TDDFT time propagation (class)
 * ============================================================================
 *
 * This file implements the rmg::tddft<OrbitalType, MatrixType> class.  The
 * pre-refactor monolithic free function RmgTddft<O,M>(...) has been split
 * into:
 *   - constructor tddft(...)        : setup, initial kick, ground-state J_0
 *   - method      tddft_md()        : outer time loop with Magnus propagator
 *   - destructor  ~tddft()          : final write, file close, free
 *   - helpers     tstconv(), gather_rho_matrix()
 *
 * Companion files:
 *   - rmg_tddft_energy.cpp   : tddft_energy_init() / tddft_energy() methods
 *   - rmg_rotate_sint.cpp    : rmg::rotate_sint() free function (Ehrenfest)
 *
 * ============================================================================
 * FORMALISM
 * ---------
 * Propagates the one-body density matrix P(t) in the basis of the ground-state
 * KS orbitals {phi_j}, which are fixed throughout the simulation.  The equation
 * of motion is the von Neumann / Liouville equation (atomic units, hbar=1):
 *
 *   i dP/dt = [H(t), P(t)]
 *
 * where P_ij(t) = sum_n f_n C_in(t) C_jn*(t) and
 *       H_ij(t) = <phi_i | H_KS[rho(t)] | phi_j>.
 *
 * This is the MO-basis analogue of the AO-density-matrix Liouville equation in
 * Gaussian-basis RT-TDDFT (e.g., NWChem, Gaussian).  Because {phi_j} are
 * orthonormal KS orbitals, there is no overlap matrix S — the equation is
 * identical to the orthogonal-basis limit of the AO form.
 *
 * At t=0:  P_ij(0) = f_i * delta_ij  (diagonal, ground-state occupations).
 * The density change is built from:
 *   rho(r,t) = rho_gnd(r) + sum_ij [P_ij(t) - f_i*delta_ij] phi_i(r) phi_j*(r)
 *
 * ALGORITHM: predictor-corrector Magnus propagator
 * -------------------------------------------------
 * Each step t -> t+dt:
 *
 *   1. PREDICTOR  H_pred(t+dt) = 2*H(t) - H(t-dt)          [linear extrap.]
 *
 *   2. CORRECTOR  (SCF until ||delta_H|| < 1e-7)
 *      a. Omega  = 0.5*(H(t) + H_pred(t+dt)) * dt           [1st Magnus term]
 *      b. P(t+dt) = exp(-i*Omega) P(t) exp(i*Omega)          [BCH series]
 *      c. rho(t+dt) from P(t+dt)                             [GetNewRho]
 *      d. V_H[rho], V_xc[rho]                                [Poisson + XC]
 *      e. H(t+dt) += <phi_i | delta_V | phi_j>               [HmatrixUpdate]
 *
 *   3. ADVANCE    P(t) <- P(t+dt),  H(t-dt) <- H(t),  H(t) <- H(t+dt)
 *
 * PERTURBATION MODES
 * ------------------
 * Two physically distinct modes correspond to finite vs. periodic systems:
 *
 *   EFIELD / POINT_CHARGE  [finite molecules, ct.is_gamma = true]
 *     External field enters as a scalar potential: V_ext = -E * r.
 *     Applied as an instantaneous kick at t=0 by adding (1/dt)*<phi_i|V|phi_j>
 *     to H.  The 1/dt factor converts the impulse to a Hamiltonian amplitude
 *     consistent with the propagator framework.
 *     Observable: dipole d(t) = integral rho(r,t) * r d^3r  [get_dipole]
 *     Data file:  basename_spin*_dipole.dat
 *
 *   VECTOR_POT  [periodic solids, multiple k-points]
 *     The position operator r is not Hermitian under PBC, so the field enters
 *     via the vector potential in the velocity gauge (atomic units):
 *       H(t) = H_KS + A(t) . p     [linear coupling, A^2 term neglected]
 *     The time-independent momentum matrix P^alpha_ij = i*vel*<phi_i|(grad+ik)|phi_j>
 *     is precomputed by VecPHmatrix + CurrentNlpp.
 *     Observable: current J_alpha(t) = Re[Tr(P(t) * P^alpha)]
 *     Data file:  basename_spin*_current.dat
 *
 * ============================================================================
 * KNOWN LIMITATIONS AND THEORY GAPS  (audited 2026-05-26: all 8 still active)
 * ============================================================================
 *
 * [GAP-1]  VECTOR_POT implements DELTA-KICK ONLY, not a sustained field.
 *   Theory says H(t) = H_KS + A(t).p with A(t) = A_0*cos(omega*t).
 *   What is implemented: A(t) = A_0*delta(t) — the kick is added once at t=0
 *   and then the system evolves freely under H_KS alone.
 *   Evidence: the time-varying update
 *     daxpy(A_0*cos(omega*t), Pxmatrix, Hmatrix_0)
 *   is present but COMMENTED OUT in the extrapolation block.
 *   ct.tddft_frequency is read from input but has no effect on the propagation.
 *   Impact: optical spectra obtained via FT of J(t) are VALID (delta-kick
 *   linear response is a standard approach), but simulating a specific laser
 *   pulse or a monochromatic CW field is NOT currently possible.
 *
 * [GAP-2]  Diamagnetic current contribution is absent.
 *   The full gauge-invariant current in velocity gauge is:
 *     J_total = J_para + J_dia
 *     J_para  = Re[Tr(P(t) * P^alpha)]          [what is computed]
 *     J_dia   = -n(r,t) * A(t)                   [MISSING]
 *   For the delta-kick (A -> 0 after t=0), J_dia = 0 and the code is correct.
 *   If GAP-1 is fixed to support sustained fields, J_dia must also be added.
 *
 * [GAP-3]  A^2 term is dropped without documentation.
 *   The minimal-coupling Hamiltonian is (p + A)^2/2 = p^2/2 + A.p + A^2/2.
 *   Only the A.p term is retained.  For weak fields this is standard, but the
 *   omission is not enforced (no amplitude check) and not stated in the input
 *   documentation.
 *
 * [GAP-4]  Non-collinear spin (noncoll) is incomplete in the VECTOR_POT path.
 *   The vtot update at STEP 2e uses only the scalar part of V_xc:
 *     vtot = vxc[0] + vh + vh_dip - old values
 *   For non-collinear spin, V_xc is a 2x2 matrix in spin space
 *   (components cx, cy, cz, in addition to the scalar part).  The off-diagonal
 *   spin components are absent from vtot.
 *   The two "noncoll need change" comments in the code mark these locations.
 *   Running VECTOR_POT with noncoll enabled will silently produce wrong H(t).
 *
 * [GAP-5]  Hmatrix_m1_cpu serves three different roles within one timestep.
 *   Before the SCF:  H(t-dt)  [used by extrapolate_Hmatrix]
 *   During SCF 2a:   Omega    [output of magnus(), overwrites H(t-dt)]
 *   During SCF 2e:   <delta_V>[output of HmatrixUpdate(), overwrites Omega]
 *   The buffer reuse is intentional but fragile: any future code that reads
 *   Hmatrix_m1_cpu after the extrapolation step will get Omega, not H(t-dt).
 *   H(t-dt) is correctly restored at STEP 3 via:
 *     Hmatrix_m1_cpu <- Hmatrix_0_cpu
 *
 * [GAP-6]  Hmatrix_cpu is never explicitly reset to H(t) at the start of each
 *   timestep — it carries the converged H from the previous step and the SCF
 *   adds INCREMENTAL potential corrections delta_V on top of it.  This is
 *   logically correct (vtot = V_new - V_old in each SCF iteration), but the
 *   convention is fragile and not documented.  Breaking the vtot = incremental
 *   convention in any future modification will silently corrupt H.
 *
 * [GAP-7]  No guard against using EFIELD mode for a periodic solid or VECTOR_POT
 *   for a finite molecule.  The two modes are physically required by the
 *   boundary conditions, but the code does not check or enforce this.  Using
 *   EFIELD with k-points or VECTOR_POT at gamma-point with is_gamma=true will
 *   compile and run without error but produce unphysical results.
 *
 * [GAP-8]  The EFIELD kick amplitude uses ct.efield_tddft_crds as the electric
 *   field E_0 (length gauge), while VECTOR_POT uses it as the vector potential
 *   amplitude A_0 (velocity gauge).  The relationship E_0 = A_0 * omega means
 *   the two modes require different input values for the same physical field
 *   strength, but this is not documented in the input file description.
 *
 * ============================================================================
 */

#include "rmg_tddft.h"
#include "../Headers/prototypes_tddft.h"
#include "GatherScatter.h"
#include "rmg_dev_allocate.h"
#include "rmg_hvector.h"
#include "blas_driver.h"
#include "rmg_reduce.h"

template <typename KpointType>
void HSmatrix (Kpoint<KpointType> *kptr, double *vtot_eig,double *vxc_psi,  KpointType *Hmat, KpointType *Smat);

void  init_point_charge_pot(double *vtot_psi, int density);
void eldyn_ort(int *desca, int Mdim, int Ndim, double *F,double *Po0,double *Po1,int *p_Ieldyn,  double *thrs,int*maxiter,  double *errmax,int
        *niter , int *p_iprint, MPI_Comm comm) ;
void eldyn_ort(int *desca, int Mdim, int Ndim, std::complex<double> *F,std::complex<double> *Po0,std::complex<double> *Po1,int *p_Ieldyn,  double *thrs,int*maxiter,  double *errmax,int
        *niter , int *p_iprint, MPI_Comm comm) ;
void eldyn_nonort(int *p_N, double *S, double *F,double *Po0,double *Pn1,int *p_Ieldyn,  double *thrs,int*maxiter,  double *errmax,int *niter , int *p_iprint) ;



template void rmg::tddft<double, double>::tddft_md(void);
template void rmg::tddft<double, std::complex<double>>::tddft_md(void);
template void rmg::tddft<std::complex<double>, std::complex<double>>::tddft_md(void);
template rmg::tddft<double, double>::~tddft(void);
template rmg::tddft<double, std::complex<double>>::~tddft(void);
template rmg::tddft<std::complex<double>, std::complex<double>>::~tddft(void);

template void rmg::tddft<double, double>::gather_rho_matrix(double *, double *);
template void rmg::tddft<double, std::complex<double>>::gather_rho_matrix(double *, std::complex<double> *);
template void rmg::tddft<std::complex<double>, std::complex<double>>::gather_rho_matrix(std::complex<double> *, std::complex<double> *);

template rmg::tddft<double, double>::tddft(spinobj<double> &vxc_in,
             fgobj<double> &vh_in,
             fgobj<double> &vnuc_in,
             spinobj<double> &rho_in,
             fgobj<double> &rhocore_in,
             fgobj<double> &rhoc_in,
             Kpoint<double> **Kptr_in);

template rmg::tddft<double, std::complex<double>>::tddft(spinobj<double> &vxc_in,
             fgobj<double> &vh_in,
             fgobj<double> &vnuc_in,
             spinobj<double> &rho_in,
             fgobj<double> &rhocore_in,
             fgobj<double> &rhoc_in,
             Kpoint<double> **Kptr_in);

template rmg::tddft<std::complex<double>, std::complex<double>>::tddft(spinobj<double> &vxc_in,
             fgobj<double> &vh_in,
             fgobj<double> &vnuc_in,
             spinobj<double> &rho_in,
             fgobj<double> &rhocore_in,
             fgobj<double> &rhoc_in,
             Kpoint<std::complex<double>> **Kptr_in);

template <typename OrbitalType, typename MatrixType>
rmg::tddft<OrbitalType, MatrixType>::tddft(spinobj<double> &vxc_in,
             fgobj<double> &vh_in,
             fgobj<double> &vnuc_in,
             spinobj<double> &rho_in,
             fgobj<double> &rhocore_in,
             fgobj<double> &rhoc_in,
             Kpoint<OrbitalType> **Kptr_in) : vxc(vxc_in), vh(vh_in), vnuc(vnuc_in),
                                            rho(rho_in), rhocore(rhocore_in), rhoc(rhoc_in)
{
    RmgTimer RT0("2-TDDFT: Initialization");
    Kptr = Kptr_in;

#if USE_NCCL
    if(pct.nccl_rank == 0)
    {
        rmg::error(ncclGetUniqueId(&ct.nccl_nd_id));
    }
    MPI_Bcast(&ct.nccl_nd_id, sizeof(ct.nccl_nd_id), MPI_BYTE, 0, pct.nccl_comm);
#if CUDA_ENABLED 
    rmg::error(cuDeviceGet( &ct.cu_dev, 0 ));
    rmg::error(cudaSetDevice(ct.cu_dev));
#endif
    rmg::error(ncclCommInitRank(&ct.nccl_local_comm, pct.nccl_comm_npes, ct.nccl_nd_id, pct.nccl_rank));
#endif  
    iprint = ct.verbose;
    FP0_BASIS = Rmg_G->get_P0_BASIS(Rmg_G->default_FG_RATIO);
    pbasis = Kptr[0]->pbasis;
    pbasis_noncoll = pbasis * ct.noncoll_factor;

    switch(ct.subdiag_driver) {

        case SUBDIAG_LAPACK:
            scalapack_groups = pct.grid_npes;
            break;
        case SUBDIAG_SCALAPACK:
            scalapack_groups = 1;
            if(ct.scalapack_block_factor >= ct.num_states - ct.tddft_start_state)
                scalapack_groups = pct.grid_npes;
            break;
        case SUBDIAG_CUSOLVER:
            scalapack_groups = pct.grid_npes;
            break;
        default:
            rmg::error("Invalid subdiag_driver type in TDDFT");

    } // end switch

    if(ct.tddft_gpu)
    {
        scalapack_groups = pct.grid_npes;
    }
    // all tddft propergating will use 1 gpu only, not sure the speed comparison with scalapack for a large system 
    int last = 1;
    numst = ct.num_states - ct.tddft_start_state; 
    MPI_Comm_size(pct.local_comm, &pct.local_comm_npes);
    numst = ( numst/pct.local_comm_npes ) * pct.local_comm_npes; 
    if(ct.tddft_tiledMM == 1)
    {
        if(!ct.tddft_gpu)
        {
            pct.local_comm = pct.grid_comm;
        }
        else
        {
            pct.local_comm = pct.nccl_comm;
        }

        MPI_Comm_size(pct.local_comm, &pct.local_comm_npes);
        MPI_Comm_rank(pct.local_comm, &pct.local_rank);

        eldyn_comm = pct.local_comm;

        // reduce the number of unoccupied states so that numst is divisible by local_comm_npes
        numst = ( numst/pct.local_comm_npes ) * pct.local_comm_npes; 
        n2 = numst * numst/pct.local_comm_npes;
        Mdim = numst;
        Ndim = numst/pct.local_comm_npes;

        Sp = new Scalapack(scalapack_groups, pct.thisimg, ct.images_per_node, numst,
            ct.scalapack_block_factor, last, pct.grid_comm);
    }
    else
    {
        Sp = new Scalapack(scalapack_groups, pct.thisimg, ct.images_per_node, numst,
            ct.scalapack_block_factor, last, pct.grid_comm);
        Mdim = Sp->GetDistMdim();
        Ndim = Sp->GetDistNdim();
        n2 = Sp->GetDistMdim() * Sp->GetDistNdim();

        if(!Sp->Participates()) n2 = 1;
        eldyn_comm =  Sp->GetComm() ;
    }
    int *desca = Sp->GetDistDesca();

    n22 = 2* n2;
    n2_C = n2 * sizeof(MatrixType)/sizeof(double);

    for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++)
    {
        Kptr[kpt]->Hmatrix_cpu     = (void *)RmgMallocHost((size_t)n2*sizeof(MatrixType));
        Kptr[kpt]->Pn0_cpu         = (void *)RmgMallocHost((size_t)n2*sizeof(double)*2);
        Kptr[kpt]->Pn1_cpu         = (void *)RmgMallocHost((size_t)n2*sizeof(double)*2);
        Kptr[kpt]->Hmatrix_m1_cpu  = (void *)RmgMallocHost((size_t)n2*sizeof(MatrixType));
        Kptr[kpt]->Hmatrix_1_cpu  = (void *)RmgMallocHost((size_t)n2*sizeof(MatrixType));
        Kptr[kpt]->Hmatrix_0_cpu   = (void *)RmgMallocHost((size_t)n2*sizeof(MatrixType));
        if(ct.tddft_mode == VECTOR_POT)
        {
            Kptr[kpt]->Pxmatrix_cpu   = (std::complex<double> *)RmgMallocHost((size_t)n2*sizeof(std::complex<double>));
            Kptr[kpt]->Pymatrix_cpu   = (std::complex<double> *)RmgMallocHost((size_t)n2*sizeof(std::complex<double>));
            Kptr[kpt]->Pzmatrix_cpu   = (std::complex<double> *)RmgMallocHost((size_t)n2*sizeof(std::complex<double>));

        }
        else
        {
            Kptr[kpt]->Akick_cpu   = (OrbitalType *)RmgMallocHost((size_t)n2*sizeof(OrbitalType));
        }
    }

    matrix_size = n2*sizeof(MatrixType);
#if CUDA_ENABLED || HIP_ENABLED
    rmg_device_pool->malloc(&Hmatrix, n2);
    rmg_device_pool->malloc(&Hmatrix_m1, n2);
    rmg_device_pool->malloc(&Hmatrix_0, n2);
    rmg_device_pool->malloc(&Hmatrix_1, n2);
    if(typeid(MatrixType) == typeid(double) )
    {
        rmg_device_pool->malloc(&Pn0, 2*n2);
        rmg_device_pool->malloc(&Pn1, 2*n2);
    }
    else
    {
        rmg_device_pool->malloc(&Pn0, n2);
        rmg_device_pool->malloc(&Pn1, n2);
    }


    size_t num_psik = ct.num_kpts_pe * ct.num_states * (size_t)pbasis_noncoll;
    rmg_device_pool->malloc(&psi_dev_pool, num_psik * 2);
    for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++)
    {
        Kptr[kpt]->psi_dev  = psi_dev_pool + 2 * kpt * ct.num_states * pbasis_noncoll;
        Kptr[kpt]->work_dev = Kptr[kpt]->psi_dev + ct.num_states * pbasis_noncoll;
        // even for tddft_floatprecision, psi_dev is still used in VecPHmatrix and CurrentNlpp
        RmgMemcpy(Kptr[kpt]->psi_dev, Kptr[kpt]->orbital_storage, ct.num_states * pbasis_noncoll * sizeof(OrbitalType));

        if(ct.tddft_floatprecision)
        {

            size_t psi_alloc = (size_t)ct.num_states * (size_t)pbasis_noncoll * sizeof(OrbitalType);
            gpuMalloc((void **)&Kptr[kpt]->psi_dev_float, psi_alloc/2);
            gpuMalloc((void **)&Kptr[kpt]->work_dev_float, psi_alloc/2);

            size_t count = (size_t)ct.num_states * (size_t)pbasis_noncoll;
            if(typeid(OrbitalType) == typeid(double))
            {
                float *work_conv = new float[count];
                CopyAndConvert(count, (double *)Kptr[kpt]->orbital_storage, work_conv);
                RmgMemcpy(Kptr[kpt]->psi_dev_float, work_conv, count * sizeof(float));
                delete [] work_conv;
            }
            else if(typeid(OrbitalType) == typeid(std::complex<double>))
            {
                std::complex<float> *work_conv = new std::complex<float>[count];
                CopyAndConvert(count, (std::complex<double> *)Kptr[kpt]->orbital_storage, work_conv);
                RmgMemcpy(Kptr[kpt]->psi_dev_float, work_conv, count * sizeof(std::complex<float>));
                delete [] work_conv;
            }

        }
    }

#else
    Pn1  = (MatrixType *)RmgMallocHost((size_t)n2*sizeof(double)*2);
    for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++)
    {
        Kptr[kpt]->work_cpu = new OrbitalType[(size_t)ct.num_states * (size_t)pbasis_noncoll];
    }
#endif

    std::vector<double> diag_elem(numst);

    if(pct.gridpe == 0) {
        printf("\n Number of states used for TDDFT: Nbasis =  %d \n",numst);
        printf(" Propagator used :  Ieldyn = %d  \\1=BCH, 2=Diagonalizer\\ \n",Ieldyn) ;
    }

    //    double *vh_x = new double[FP0_BASIS];
    //    double *vh_y = new double[FP0_BASIS];
    //    double *vh_z = new double[FP0_BASIS];



    get_dipole(rho.data(), rhoc.data(), dipole_tot);
    if(ct.dipole_corr[0]+ct.dipole_corr[1]+ct.dipole_corr[2] >0)
    {
        DipoleCorrection(dipole_tot,  vh_dipole.data());
    }

    efactor = ct.energy_output_conversion[ct.energy_output_units];
    eunits = ct.energy_output_string[ct.energy_output_units].c_str();

    if(!ct.norm_conserving_pp)
    {
        rmg::error(" \n  TDDFT support NCPP only \n");
    }

    if(pct.kstart == 0 && pct.gridpe == 0)
    {


        if(ct.tddft_mode == VECTOR_POT)
        {
            filename = std::string(ct.basename)+"_spin" +std::to_string(pct.spinpe)+ "_current.dat";

            current_fi = fopen(filename.c_str(), "w");
            fprintf(current_fi, "\n  &&electric field in cartesian unit:  %e  %e  %e ",ct.efield_tddft_crds[0], ct.efield_tddft_crds[1], ct.efield_tddft_crds[2]);

            if(ct.BerryPhase)
            {
                filename = std::string(ct.basename) +"_spin" +std::to_string(pct.spinpe)+ "_bp_dipole.dat";
                dbp_fi = fopen(filename.c_str(), "w");
                fprintf(dbp_fi, "\n  &&electric field in cartesian unit:  %e  %e  %e ",ct.efield_tddft_crds[0], ct.efield_tddft_crds[1], ct.efield_tddft_crds[2]);
            }
        }
        else
        {
            filename = std::string(ct.basename) +"_spin" +std::to_string(pct.spinpe)+ "_dipole.dat";

            dfi = fopen(filename.c_str(), "w");

            fprintf(dfi, "\n  &&electric field in cartesian unit:  %e  %e  %e ",ct.efield_tddft_crds[0], ct.efield_tddft_crds[1], ct.efield_tddft_crds[2]);
        }
    }


    double time_step = ct.tddft_time_step;

    ReadData (ct.infile, vh.data(), rho_ground.data(), vxc.data(), Kptr);
    rho_ground.get_oppo();

    if(ct.tddft_energy)
    {

        tddft_energy_init(vxc, vh, vnuc, rho_ground, rhocore, rhoc, Kptr, *Sp, Mdim, Ndim, Eterms_ground);
        for(int i = 0; i < 6; i++) Eterms[i] = Eterms_ground[i];
        if(pct.kstart == 0 && pct.gridpe == 0 && pct.spinpe == 0)
        {
            filename = std::string(ct.basename) + "_totalE";
            efi = fopen(filename.c_str(), "w");

            fprintf(efi, " && totalE_0, EkinPseudo_0, Vh_0, Exc_0  II  Edownfold%s", eunits);
            fprintf(efi, "\n&& %16.8e  %16.8e  %16.8e  %16.8e %16.8e %16.8e at Ground state", Eterms_ground[0], 
                    Eterms_ground[1],Eterms_ground[2],Eterms_ground[3],Eterms_ground[4],Eterms_ground[5]);
        }
    }

    if(ct.restart_tddft)
    {

        for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++)
        {
            int kpt_glob = kpt + pct.kstart;

            std::string ofile = std::format("{}_spin{}_kpt{}_gridpe{}", ct.infile_tddft, pct.spinpe, kpt_glob, pct.gridpe);
            ReadData_rmgtddft(ofile.c_str(), vh.data(), vxc.data(), vh_dipole.data(), (double *)Kptr[kpt]->Pn0_cpu, (double *)Kptr[kpt]->Hmatrix_cpu, 
                    (double *)Kptr[kpt]->Hmatrix_m1_cpu, (double *)Kptr[kpt]->Hmatrix_0_cpu, 
                    &pre_steps, n2, n2_C, numst);
        }
    }
    else
    {

        for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++) {

            for(int i = 0; i < numst; i++) diag_elem[i] = Kptr[kpt]->Kstates[i + ct.tddft_start_state].eig[0];

            double one = 1.0;
            memset(Kptr[kpt]->Hmatrix_cpu, 0, n2*sizeof(MatrixType));
            MatDiagSet((MatrixType *)Kptr[kpt]->Hmatrix_cpu, diag_elem, one, numst, *Sp);

            memcpy(Kptr[kpt]->Hmatrix_1_cpu, Kptr[kpt]->Hmatrix_cpu, matrix_size);
            memcpy(Kptr[kpt]->Hmatrix_0_cpu, Kptr[kpt]->Hmatrix_cpu, matrix_size);
            memcpy(Kptr[kpt]->Hmatrix_m1_cpu, Kptr[kpt]->Hmatrix_0_cpu, matrix_size);


            for(int i = 0; i < numst; i++) diag_elem[i] =  Kptr[kpt]->Kstates[i + ct.tddft_start_state].occupation[0];
            memset(Kptr[kpt]->Pn0_cpu, 0, 2*n2*sizeof(double));
            MatDiagSet((MatrixType *)Kptr[kpt]->Pn0_cpu, diag_elem, one, numst, *Sp);
        }

    }

    //   if(pct.gridpe == 0)
    //   for(int i = 0; i < 5; i++) 
    //   { printf("Akick\n");
    //       for(int j = 0; j < 5; j++) printf(" %10.4e", Akick[i*numst + j]);
    //   }


    if(ct.tddft_mode == EFIELD || ct.tddft_mode == POINT_CHARGE)
    {
        double alpha = 1.0/time_step;
        for (int idx = 0; idx < FP0_BASIS; idx++) vtot[idx] = 0.0;
        if(ct.tddft_mode == EFIELD)
        {
            init_efield(vtot.data(), ct.efield_tddft_crds);
            GetVtotPsi (vtot_psi.data(), vtot.data(), Rmg_G->default_FG_RATIO);
        }
        else if(ct.tddft_mode == POINT_CHARGE)
        {
            init_point_charge_pot(vtot_psi.data(), 1);
        }
        for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++) {
            HmatrixUpdate<OrbitalType, OrbitalType, OrbitalType>(Kptr[kpt], vtot_psi, vxc_psi, (OrbitalType *)Kptr[kpt]->Akick_cpu, ct.tddft_start_state, numst, desca);
            daxpy ( &n2_C,  &alpha, (double *)Kptr[kpt]->Akick_cpu, &ione , (double *)Kptr[kpt]->Hmatrix_0_cpu , &ione) ;
            memcpy(Kptr[kpt]->Hmatrix_m1_cpu, Kptr[kpt]->Hmatrix_0_cpu, matrix_size);
        }
    }

    //  initialize   data for rt-td-dft
    //int nblock = 10 ;   //  size of tthe block for printing (debug!)

    /*
       if(pct.gridpe == 0) { printf("**** Hmat  : \n");  print_matrix_d(Hmatrix,   &nblock, &numst)   ; }
       if(pct.gridpe == 0) { printf("**** Hmat0 : \n");  print_matrix_d(Hmatrix_0, &nblock, &numst)   ; }
       if(pct.gridpe == 0) { printf("**** Hmat1 : \n");  print_matrix_d(Hmatrix_1, &nblock, &numst)   ; }
     */

    current[0] = 0.0;
    current[1] = 0.0;
    current[2] = 0.0;
    current0[0] = 0.0;
    current0[1] = 0.0;
    current0[2] = 0.0;
    tot_bp_pol = 0.0;
    if(ct.tddft_mode == VECTOR_POT)
    {
        // ====================================================================
        // VELOCITY GAUGE INITIALIZATION (run once before the time loop)
        //
        // THEORY: why velocity gauge for periodic systems
        // -----------------------------------------------
        // For a crystal, the position operator r is not Hermitian under PBC
        // (Bloch states extend over all space).  The electric field therefore
        // cannot enter as V = -e*E*r (length gauge).  Instead, the field is
        // coupled through the vector potential A(t) in the minimal-coupling
        // Hamiltonian (atomic units, linear-in-A approximation):
        //
        //   H(t) = H_KS  +  A(t) · p                 [velocity gauge]
        //
        // where  p = -i*grad  is the canonical momentum operator.
        // For a monochromatic field:  A(t) = A_0 * epsilon_hat * cos(omega*t)
        // For a delta-kick at t=0:   A(t) = A_0 * epsilon_hat * delta(t)
        //
        // Here ct.efield_tddft_crds[0,1,2] = A_0 * epsilon_hat  (amplitude vector).
        //
        // STEP A — Build the time-independent momentum matrix P^alpha_ij
        // ----------------------------------------------------------------
        // VecPHmatrix computes for each Cartesian direction alpha:
        //
        //   P^alpha_ij = i * vel * <phi_i | d/dx_alpha + i*k_alpha | phi_j>
        //
        // where:
        //   - vel = Omega/N  is the real-space grid integration weight
        //   - the +ik_alpha term is the k-correction for Bloch states
        //   - the factor i converts grad to the momentum operator  p = -i*grad
        //
        // Result stored in: Pxmatrix_cpu, Pymatrix_cpu, Pzmatrix_cpu  (N×N complex)
        //
        // STEP B — Add the delta-kick to H at t=0
        // -----------------------------------------
        // For an instantaneous kick at t=0, A(t) = A_0 * delta(t), the
        // integrated effect on the Hamiltonian over the first timestep is:
        //
        //   H(t=0) += A_0 · (eps_x * P^x + eps_y * P^y + eps_z * P^z)
        //            = sum_alpha  A_0_alpha * P^alpha
        //
        // [cos(omega*t=0) = 1, so the time-varying field evaluates to A_0 at t=0]
        //
        // After the kick H contains the full velocity-gauge Hamiltonian at t=0.
        //
        // STEP C — Add NL-PP correction to the current operator
        // -------------------------------------------------------
        // CurrentNlpp adds the nonlocal pseudopotential contribution to
        // Pxmatrix/Pymatrix/Pzmatrix (see CurrentNlpp.cpp for theory).
        // After this, P^alpha contains the TOTAL current operator matrix.
        // ====================================================================
        if(ct.verbose) {
            rmg::printlog("\n starting VecP matrix ");
            fflush(NULL);
        }
        for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++) {
            // STEP A: momentum matrix P^alpha_ij = i*vel*<phi_i|(grad+ik)|phi_j>
            VecPHmatrix(Kptr[kpt], ct.efield_tddft_crds, desca, ct.tddft_start_state, numst);

            if(pre_steps == 0)
            {
                // STEP B: delta-kick at t=0 — add A_0·P to H(t=0) and H(t=-dt)
                // H_ij(t=0) += sum_alpha  A_0_alpha * P^alpha_ij
                // cos(omega*0) = 1, so the full A_0 amplitude is applied here.
                daxpy ( &n2_C ,  &ct.efield_tddft_crds[0], (double *)Kptr[kpt]->Pxmatrix_cpu, &ione , (double *)Kptr[kpt]->Hmatrix_m1_cpu,  &ione) ;
                daxpy ( &n2_C ,  &ct.efield_tddft_crds[1], (double *)Kptr[kpt]->Pymatrix_cpu, &ione , (double *)Kptr[kpt]->Hmatrix_m1_cpu,  &ione) ;
                daxpy ( &n2_C ,  &ct.efield_tddft_crds[2], (double *)Kptr[kpt]->Pzmatrix_cpu, &ione , (double *)Kptr[kpt]->Hmatrix_m1_cpu,  &ione) ;
                memcpy(Kptr[kpt]->Hmatrix_0_cpu, Kptr[kpt]->Hmatrix_m1_cpu, matrix_size);
            }

            // STEP C: NL-PP correction — adds i[V_NL, r_alpha] to P^alpha_ij
            CurrentNlpp(Kptr[kpt], desca, ct.tddft_start_state, numst);

            if(0)
            {
                for(int i = 0; i < n2; i++) 
                {
                    Kptr[kpt]->Pxmatrix_cpu[i] = 0.0;
                    Kptr[kpt]->Pymatrix_cpu[i] = 0.0;
                    Kptr[kpt]->Pzmatrix_cpu[i] = 0.0;
                }
                CurrentOperator(Kptr[kpt], desca, ct.tddft_start_state);
            }
        }
        if(ct.verbose) {
            rmg::printlog("\n done VecP matrix ");
            fflush(NULL);
        }

        // GROUND-STATE CURRENT J_0 (reference baseline)
        // -----------------------------------------------
        // Computes the paramagnetic current density at t=0 (before the kick).
        // For a time-reversal-symmetric ground state this should be zero.
        // A nonzero J_0 arises only from k-point sampling asymmetry; it is
        // subtracted as a baseline when interpreting the time-dependent signal.
        //
        //   J_alpha(t=0) = sum_k  w_k * Re[ Tr_ij( P_ij(t=0) * P^alpha_ij ) ]
        //                = sum_k  w_k * Re[ P(t=0) : P^alpha ]  (Frobenius inner product)
        //
        // The zdotc call contracts the full N^2 matrices element-by-element:
        //   Re[ sum_ij  conj(P_ij(t=0)) * P^alpha_ij ]
        for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++) {
            std::complex<double> tem_x = rmg_zdotc(&n2, (std::complex<double> *)Kptr[kpt]->Pn0_cpu, &ione, (std::complex<double> *)Kptr[kpt]->Pxmatrix_cpu, &ione);
            current0[0] += std::real(tem_x) * Kptr[kpt]->kp.kweight;
            std::complex<double> tem_y = rmg_zdotc(&n2, (std::complex<double> *)Kptr[kpt]->Pn0_cpu, &ione, (std::complex<double> *)Kptr[kpt]->Pymatrix_cpu, &ione);
            current0[1] += std::real(tem_y) * Kptr[kpt]->kp.kweight;
            std::complex<double> tem_z = rmg_zdotc(&n2, (std::complex<double> *)Kptr[kpt]->Pn0_cpu, &ione, (std::complex<double> *)Kptr[kpt]->Pzmatrix_cpu, &ione);
            current0[2] += std::real(tem_z) * Kptr[kpt]->kp.kweight;
        }
        if(ct.BerryPhase)
        {
            // Rmg_BP->CalcBP_Skk1(Kptr, ct.tddft_start_state, matrix_glob, *Sp);
            // Rmg_BP->CalcBP_tddft(Kptr, tot_bp_pol, matrix_glob, *Sp);
            Rmg_BP->tddft_Xml(Kptr, ct.tddft_start_state, *Sp);
            tot_bp_pol = 0.0;
            for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++) {
                std::complex<double> tem_x = rmg_zdotc(&n2, (std::complex<double> *)Kptr[kpt]->Pn0_cpu, &ione, (std::complex<double> *)Kptr[kpt]->BP_Xml, &ione);
                tot_bp_pol += std::real(tem_x) * Kptr[kpt]->kp.kweight;
            }
            MPI_Allreduce(MPI_IN_PLACE, &tot_bp_pol, 1, MPI_DOUBLE, MPI_SUM, pct.kpsub_comm);
            MPI_Allreduce(MPI_IN_PLACE, &tot_bp_pol, 1, MPI_DOUBLE, MPI_SUM, eldyn_comm);
        }
    }

    MPI_Allreduce(MPI_IN_PLACE, current0, 3, MPI_DOUBLE, MPI_SUM, pct.kpsub_comm);
    MPI_Allreduce(MPI_IN_PLACE, current0, 3, MPI_DOUBLE, MPI_SUM, eldyn_comm);
    Rmg_Symm->symm_vec(current0);

#if CUDA_ENABLED || HIP_ENABLED
    if(ct.tddft_floatprecision)
    {
        for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++)
        {

            // psi_dev is no longer needed, so psi_dev_float and work_dev_float use these memory
            Kptr[kpt]->psi_dev_float = Kptr[kpt]->psi_dev;
            Kptr[kpt]->work_dev_float = Kptr[kpt]->work_dev;

            size_t count = (size_t)ct.num_states * (size_t)pbasis_noncoll;
            if(typeid(OrbitalType) == typeid(double))
            {
                float *work_conv = new float[count];
                CopyAndConvert(count, (double *)Kptr[kpt]->orbital_storage, work_conv);
                RmgMemcpy(Kptr[kpt]->psi_dev_float, work_conv, count * sizeof(float));
                delete [] work_conv;
            }
            else if(typeid(OrbitalType) == typeid(std::complex<double>))
            {
                std::complex<float> *work_conv = new std::complex<float>[count];
                CopyAndConvert(count, (std::complex<double> *)Kptr[kpt]->orbital_storage, work_conv);
                RmgMemcpy(Kptr[kpt]->psi_dev_float, work_conv, count * sizeof(std::complex<float>));
                delete [] work_conv;
            }

        }
    }
#endif


    if(pct.kstart == 0 && pct.gridpe == 0)
    {
        if(ct.tddft_mode == VECTOR_POT)
        {
            fprintf(current_fi, "\n  &&current at groud state:  %18.10e  %18.10e  %18.10e nonzero due to kpoint sampling",
                    current0[0], current0[1], current0[2]);
        }
        else
        {
            fprintf(dfi, "\n  &&dipole at groud state:  %18.10e  %18.10e  %18.10e ",
                    dipole_tot[0], dipole_tot[1], dipole_tot[2]);
        }
        if(ct.BerryPhase)
        {
            fprintf(dbp_fi, "\n  &&dipole at groud state BeryPhase (C/m^2):  %18.10e  %18.10e  %18.10e ",
                    tot_bp_pol, 0.0,0.0);
        }
        fflush(NULL);
    }

}

    // TDDFT MD loop
template <typename OrbitalType, typename MatrixType>
void rmg::tddft<OrbitalType, MatrixType>::tddft_md(void)
{

    RmgTimer *RT2a;
    Kpoint<double> *kptr_d;
    Kpoint<std::complex<double>> *kptr_c;
    int ij_err=0;
    double vtxc, etxc;
    int *desca = Sp->GetDistDesca();
    static double total_time = 0.0;
    spinobj<double> trho;
    trho.set(0.0);

    //get_vxc(rho, rho_oppo, rhocore, vxc);
    RmgTimer *RT1 = new RmgTimer("2-TDDFT: exchange/correlation");
    //vxc_in = vxc;
    compute_vxc(rho.data(), rhocore.data(), etxc, vtxc, vxc.data(), ct.nspin);
    GetVtotPsi(vxc_psi.data(), vxc.data(), Rmg_G->default_FG_RATIO);
    delete RT1;

    RT1 = new RmgTimer("2-TDDFT: Vh");
    //vh_in = vh;
    VhDriver(rho.data(), rhoc.data(), vh.data(), ct.vh_ext, 1.0-12);
    delete RT1;

    for (int idx = 0; idx < vtot.pbasis; idx++)
    {   
	vtot[idx] = vxc[idx] + vh[idx] + vnuc[idx];
    }
    GetVtotPsi(vtot_psi.data(), vtot.data(), Rmg_G->default_FG_RATIO);

    rmg::hvector<OrbitalType> Hmat(numst*numst), Smat(numst*numst);
    rmg::hvector<MatrixType> Hmat_mtype(numst*numst);
    // ====================================================================
    // PER-CALL BOOTSTRAP: rebuild sint, and (after the first call) H
    //
    // tddft_md() is invoked once per MD step in the outer Ehrenfest loop.
    // Between calls the ions may have moved, so:
    //
    //   (a) BetaProjector->project() always runs and refreshes sint =
    //       <beta_lm | phi_j> for the current geometry.  Needed by the
    //       nonlocal-PP machinery in every subsequent operation.
    //
    //   (b) The `if(first_step)` block (note: first_step is initialized
    //       to 0 and incremented AFTER the loop, so this block is SKIPPED
    //       on the very first invocation and RUNS on every later call):
    //       HSmatrix rebuilds H_ij = <phi_i | H_KS | phi_j> from scratch
    //       in the current geometry, then DistributeMatrix scatters it
    //       into the BLACS-distributed Hmatrix_cpu storage.  All three
    //       buffers (H_m1, H_0, H_1) are re-seeded with this fresh H so
    //       the predictor-corrector loop has consistent inputs.
    //
    //   The first call is skipped because the constructor already built
    //   H as the diagonal of ground-state eigenvalues — valid only as
    //   long as ions have not moved.
    // ====================================================================
    // Recompute sint arrays which is necessary for dynamics.
    static int first_step;
    for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++)
    {
#if HIP_ENABLED || CUDA_ENABLED
        this->Kptr[kpt]->BetaProjector->project(Kptr[kpt], Kptr[kpt]->newsint_local, 0,
                Kptr[kpt]->nstates * ct.noncoll_factor, Kptr[kpt]->nl_weight_gpu);
#else
        this->Kptr[kpt]->BetaProjector->project(Kptr[kpt], Kptr[kpt]->newsint_local, 0,
                Kptr[kpt]->nstates * ct.noncoll_factor, Kptr[kpt]->nl_weight);
#endif
        if(first_step)
        {
            HSmatrix (this->Kptr[kpt], vtot_psi.data(), vxc_psi.data(),  Hmat.data(), Smat.data());
            MatrixType *hptr = (MatrixType *)this->Kptr[kpt]->Hmatrix_cpu;
            for(int i = 0; i < numst * numst; i++) Hmat_mtype[i] = Hmat[i];
            this->Sp->DistributeMatrix(Hmat_mtype.data(), hptr);
            memcpy(Kptr[kpt]->Hmatrix_1_cpu, Kptr[kpt]->Hmatrix_cpu, matrix_size);
            memcpy(Kptr[kpt]->Hmatrix_0_cpu, Kptr[kpt]->Hmatrix_cpu, matrix_size);
            memcpy(Kptr[kpt]->Hmatrix_m1_cpu, Kptr[kpt]->Hmatrix_0_cpu, matrix_size);
            if(ct.tddft_energy)
            {
                tddft_energy_init(vxc, vh, vnuc, rho, rhocore, rhoc, Kptr, *Sp, Mdim, Ndim, Eterms);
            }
        }
    }

    first_step++;

    //  run rt-td-dft
    //
    // ========================================================================
    // OUTER TIME LOOP  t = n*dt,  n = 0, 1, ..., tddft_steps-1
    //   Advances the density matrix P(t) by one timestep dt using a
    //   predictor-corrector Magnus propagator with SCF self-consistency.
    //   State variables carried across steps:
    //     Pn0        = P(t)       density matrix at current time
    //     Hmatrix_0  = H(t)       KS Hamiltonian matrix at current time
    //     Hmatrix_m1 = H(t-dt)    KS Hamiltonian matrix one step back
    // ========================================================================
    for(int tddft_steps = 0; tddft_steps < ct.tddft_steps; tddft_steps++)
    {
        //if(pct.gridpe == 0) printf("=========================================================================\n   step:  %d\n", tddft_steps);

        tot_steps = pre_steps + tddft_steps;

        //  guess H1 from  H(0) and H(-1):

        current[0] = 0.0;
        current[1] = 0.0;
        current[2] = 0.0;

        // ----------------------------------------------------------------
        // STEP 1 — PREDICTOR: extrapolate H(t+dt) from H(t) and H(t-dt)
        //   H_pred(t+dt) = 2*H(t) - H(t-dt)   [linear extrapolation]
        //   Result stored in Hmatrix_1 (= H_pred(t+dt)), used as the
        //   initial guess for the SCF loop below.
        // ----------------------------------------------------------------
        RT2a = new RmgTimer("2-TDDFT: extrapolate");
        for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++) {
            //if(ct.tddft_mode == VECTOR_POT && tot_steps == 0)
            //{
            //    double coswt = cos(ct.tddft_frequency * tot_steps * time_step);
            //    double coswtx = coswt * ct.efield_tddft_crds[0];
            //    double coswty = coswt * ct.efield_tddft_crds[1];
            //    double coswtz = coswt * ct.efield_tddft_crds[2];
            //    daxpy ( &n2_C ,  &coswtx, (double *)Kptr[kpt]->Pxmatrix_cpu, &ione , (double *)Kptr[kpt]->Hmatrix_0_cpu,  &ione) ;
            //    daxpy ( &n2_C ,  &coswty, (double *)Kptr[kpt]->Pymatrix_cpu, &ione , (double *)Kptr[kpt]->Hmatrix_0_cpu,  &ione) ;
            //    daxpy ( &n2_C ,  &coswtz, (double *)Kptr[kpt]->Pzmatrix_cpu, &ione , (double *)Kptr[kpt]->Hmatrix_0_cpu,  &ione) ;
            //}
            extrapolate_Hmatrix ((double *)Kptr[kpt]->Hmatrix_m1_cpu, (double *)Kptr[kpt]->Hmatrix_0_cpu, (double *)Kptr[kpt]->Hmatrix_1_cpu, n2_C) ;
        }   

        rmg::sync_device();
        delete RT2a;


        int  Max_iter_scf = 10 ; int  iter_scf =0 ;
        double err =1.0e0   ;
        thrs_dHmat  = 1e-7  ;

        double  thrs_bch =1.0e-7; 
        int     maxiter_bch  =100;
        double  errmax_bch ;
        int     niter_bch ;


        // ----------------------------------------------------------------
        // STEP 2 — CORRECTOR: SCF loop until H(t+dt) is self-consistent
        //   Iterates until ||H_new - H_old||_inf < thrs_dHmat (= 1e-7).
        //   Each iteration: propagate P with current H_pred, rebuild rho,
        //   update V_H and V_xc, recompute H(t+dt), check convergence.
        // ----------------------------------------------------------------
        //-----   SCF loop  starts here:
        while (err > thrs_dHmat &&  iter_scf <  Max_iter_scf)  {

            for(int idx = 0; idx < FP0_BASIS; idx++) rho_ksum[idx] = 0.0;
            for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++) {
                RT2a = new RmgTimer("2-TDDFT: memcpy");
                if(ct.tddft_gpu)
                {
                    RmgMemcpy(Hmatrix, Kptr[kpt]->Hmatrix_cpu, matrix_size);
                    RmgMemcpy(Hmatrix_m1, Kptr[kpt]->Hmatrix_m1_cpu, matrix_size);
                    RmgMemcpy(Hmatrix_1, Kptr[kpt]->Hmatrix_1_cpu, matrix_size);
                    RmgMemcpy(Hmatrix_0, Kptr[kpt]->Hmatrix_0_cpu, matrix_size);
                    RmgMemcpy(Pn0, Kptr[kpt]->Pn0_cpu, 2*n2*sizeof(double));
                }
                else
                {
                    Hmatrix = (MatrixType *)Kptr[kpt]->Hmatrix_cpu;
                    Hmatrix_m1 = (MatrixType *)Kptr[kpt]->Hmatrix_m1_cpu;
                    Hmatrix_1 = (MatrixType *)Kptr[kpt]->Hmatrix_1_cpu;
                    Hmatrix_0 = (MatrixType *)Kptr[kpt]->Hmatrix_0_cpu;
                    Pn0 = (MatrixType *)Kptr[kpt]->Pn0_cpu;
                    Pn1 = (MatrixType *)Kptr[kpt]->Pn1_cpu;
                }
                rmg::sync_device();
                delete RT2a;
                RT2a = new RmgTimer("2-TDDFT: ELDYN");
                if(ct.verbose) {
                    rmg::printlog("\n start magnus and eldyn ");
                    fflush(NULL);
                }
                // STEP 2a — MAGNUS OPERATOR: build the propagator exponent Omega
                //   Omega = 0.5*(H(t) + H_pred(t+dt)) * dt
                //   This is the 1st-order Magnus approximation to Int_t^{t+dt} H(tau)dtau.
                //   Inputs:  Hmatrix_0 = H(t),  Hmatrix_1 = H_pred(t+dt)
                //   Output:  Hmatrix_m1 = Omega  (reuses the m1 buffer)
                magnus ((double *)Hmatrix_0,    (double *)Hmatrix_1 , time_step, (double *)Hmatrix_m1 , n2_C) ;
                /* --- C++  version:  --*/

                // STEP 2b — PROPAGATE DENSITY MATRIX via BCH expansion
                //   P(t+dt) = exp(-i*Omega) * P(t) * exp(i*Omega)
                //           = sum_k (1/k!) (-i)^k [Omega,[...,[Omega,P(t)]...]]
                //   P is stored split as [S | A] with S=Re(P), A=Im(P) (size 2*N^2).
                //   Ieldyn=1: BCH/commutator series (commutp) until convergence.
                //   Inputs:  Hmatrix_m1=Omega, Pn0=P(t)
                //   Output:  Pn1 = P(t+dt)
                eldyn_ort(desca, Mdim, Ndim,  Hmatrix_m1,Pn0,Pn1,&Ieldyn, &thrs_bch,&maxiter_bch,  &errmax_bch,&niter_bch ,  &iprint, eldyn_comm) ;
                RmgMemcpy(Kptr[kpt]->Pn1_cpu, Pn1, 2*n2*sizeof(double));

                if(ct.verbose) {
                    rmg::printlog("\n done magnus and eldyn ");
                    fflush(NULL);
                }
                delete(RT2a);

                // if(pct.gridpe == 0) { printf("**** Pn1 : \n");   print_matrix_z(Pn1,  &nblock, &numst)  ; }

                //            for(i = 0; i < 10; i++) 
                //            { printf("Pn\n");
                //           for(int j = 0; j < 10; j++) printf(" %8.1e", i, Pn1[i*numst + j]);
                //          }


                /////// <----- update Hamiltonian from  Pn1

                // STEP 2c — DENSITY UPDATE: build rho(r) from P(t+dt)
                //   rho(r,t+dt) = rho_gnd(r) + Delta_rho(r)
                //   Delta_rho(r) = sum_ij [P_ij(t+dt) - f_i*delta_ij] * phi_i(r)*phi_j*(r)
                //   where phi_i are the (fixed) ground-state KS orbitals.
                //   For periodic systems: rho = sum_k w_k * rho_k (BZ sum below).
                RT2a = new RmgTimer("2-TDDFT: Rho");
                rmg::sync_device();
                if(ct.verbose) {
                    rmg::printlog("\n start rho calc ");
                    fflush(NULL);
                }
                if(ct.tddft_floatprecision)
                {
                    if(ct.is_gamma)
                    {
                        kptr_d = (Kpoint<double> *)Kptr[kpt];
                        GetNewRho_rmgtddft<double, float, MatrixType>(kptr_d, rho_k, Pn1, numst, ct.tddft_start_state, *Sp);
                    }
                    else
                    {
                        kptr_c = (Kpoint<std::complex<double>> *)Kptr[kpt];
                        GetNewRho_rmgtddft<std::complex<double>, std::complex<float>, std::complex<double> >(kptr_c, rho_k, (std::complex<double> *)Pn1, numst, ct.tddft_start_state, *Sp);
                    }
                }
                else
                {
                    GetNewRho_rmgtddft<OrbitalType, OrbitalType, MatrixType>(Kptr[kpt], rho_k, Pn1, numst, ct.tddft_start_state, *Sp);
                }

                if(ct.verbose) {
                    rmg::printlog("\n done rho calc ");
                    fflush(NULL);
                }
                int kpt_glob = kpt + pct.kstart;
                for(int idx = 0; idx < FP0_BASIS; idx++) rho_ksum[idx] += rho_k[idx] * ct.kp[kpt_glob].kweight;

                delete(RT2a);

            }

            MPI_Allreduce(MPI_IN_PLACE, rho_ksum.data(), FP0_BASIS, MPI_DOUBLE, MPI_SUM, pct.kpsub_comm);
            for(int idx = 0; idx < FP0_BASIS; idx++) rho[idx] = rho_ksum[idx] + rho_ground[idx];
            rho.get_oppo();

            //write_rho_x(rho, "update rho");
            //write_rho_x(rho_ground.data(), "groumd rho");

            //dcopy(&FP0_BASIS, vh_dipole.data(), &ione, vh_dipole_old.data(), &ione);
            //dcopy(&FP0_BASIS, vh.data(), &ione, vh_old.data(), &ione);
            //dcopy(&FP0_BASIS, vxc.data(), &ione, vxc_old.data(), &ione);
            vh_old = vh;
            vh_dipole_old = vh_dipole;
            vxc_old = vxc;

            // STEP 2d — UPDATE POTENTIALS from new rho(t+dt)
            //   V_xc[rho(t+dt)] via LDA/GGA functional
            //   V_H [rho(t+dt)] via Poisson equation (VhDriver)
            //get_vxc(rho, rho_oppo, rhocore, vxc);
            RmgTimer *RT1 = new RmgTimer("2-TDDFT: exchange/correlation");
            //vxc_in = vxc;
            compute_vxc(rho.data(), rhocore.data(), etxc, vtxc, vxc.data(), ct.nspin);
            delete RT1;

            RT1 = new RmgTimer("2-TDDFT: Vh");
            //vh_in = vh;
            VhDriver(rho.data(), rhoc.data(), vh.data(), ct.vh_ext, 1.0-12);
            delete RT1;

            get_dipole(rho.data(), rhoc.data(), dipole_tot);
            if(ct.dipole_corr[0]+ct.dipole_corr[1]+ct.dipole_corr[2] >0)
            {
                DipoleCorrection(dipole_tot,  vh_dipole.data());
            }

            // STEP 2e — UPDATE HAMILTONIAN MATRIX H(t+dt)
            //   Compute the change in the effective potential:
            //     Delta_V(r) = [V_xc + V_H + V_dip](t+dt) - [same](t)
            //   Interpolate Delta_V from the fine grid to the wavefunction grid.
            //   Then add its matrix elements to H:
            //     H_ij(t+dt) = H_ij(t) + <phi_i | Delta_V | phi_j>
            //   HmatrixUpdate computes the <phi_i|V|phi_j> overlap via GEMM:
            //     A_ij = vel * sum_r phi_i*(r) * V(r) * phi_j(r)
            //   where vel = Omega/N is the real-space integration weight.
            // noncoll need change
            for (int idx = 0; idx < FP0_BASIS; idx++) {
                vtot[idx] = vxc[idx] + vh[idx] + vh_dipole[idx]
                    -vxc_old[idx] -vh_old[idx] - vh_dipole_old[idx];
            }

            vxc_diff = vxc - vxc_old;


            //noncoll need change
            GetVtotPsi (vtot_psi.data(), vtot.data(), Rmg_G->default_FG_RATIO);

            for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++) {
                RT2a = new RmgTimer("2-TDDFT: Hupdate");
                if(ct.tddft_floatprecision)
                {
                    if(ct.is_gamma)
                    {
                        kptr_d = (Kpoint<double> *)Kptr[kpt];
                        HmatrixUpdate<double,float, MatrixType>  (kptr_d, vtot_psi, vxc_psi, (MatrixType *)Kptr[kpt]->Hmatrix_m1_cpu, ct.tddft_start_state, numst, desca);                                     
                    }
                    else
                    {
                        kptr_c = (Kpoint<std::complex<double>> *)Kptr[kpt];
                        HmatrixUpdate<std::complex<double>,std::complex<float>, std::complex<double>>  (kptr_c, vtot_psi, vxc_psi, (std::complex<double> *)Kptr[kpt]->Hmatrix_m1_cpu, ct.tddft_start_state, numst, desca);                                     
                    }
                }
                else
                {
                    HmatrixUpdate<OrbitalType,OrbitalType, MatrixType>  (Kptr[kpt], vtot_psi, vxc_psi, (MatrixType *)Kptr[kpt]->Hmatrix_m1_cpu, ct.tddft_start_state, numst, desca);                                     
                }
                delete(RT2a);

                RT2a = new RmgTimer("2-TDDFT: conv check");
                rmg::sync_device();
                double one = 1.0, mone = -1.0;
                daxpy( &n2_C ,  &one, (double *)Kptr[kpt]->Hmatrix_m1_cpu, &ione , (double *)Kptr[kpt]->Hmatrix_cpu,  &ione) ;

                //   End of Hamiltonian update: Hmatrix_cpu now holds H(t+dt)

                // CONVERGENCE CHECK: ||H_new - H_old||_inf
                //   Hmatrix_1 temporarily holds (H_new - H_old) for the norm test.
                //   If converged, Hmatrix_1 is updated to H_new for the next SCF iter.
                rmg::sync_device();
                daxpy ( &n2_C ,  &mone, (double *)Kptr[kpt]->Hmatrix_cpu, &ione , (double *)Kptr[kpt]->Hmatrix_1_cpu ,  &ione) ;

                //tst_conv_matrix (&err, &ij_err ,  Hmatrix_1,  n2, Sp->GetComm()) ;  //  check error  how close  H and H_old are

                bool tConv;
                tstconv((double *)Kptr[kpt]->Hmatrix_1_cpu, &n2_C, &thrs_dHmat,&ij_err,&err,&tConv, eldyn_comm);
                memcpy(Kptr[kpt]->Hmatrix_1_cpu, Kptr[kpt]->Hmatrix_cpu, matrix_size);
                delete(RT2a);
            }

            MPI_Allreduce(MPI_IN_PLACE, &err, 1, MPI_DOUBLE, MPI_MAX, pct.kpsub_comm);
            MPI_Allreduce(MPI_IN_PLACE, &err, 1, MPI_DOUBLE, MPI_MAX, pct.spin_comm);


            if(pct.imgpe == 0) { printf("step: %5d  iteration: %d  thrs= %12.5e err=  %12.5e at element: %5d \n", 
                    tddft_steps, iter_scf,    thrs_dHmat,  err,         ij_err); } 
            rmg::printlog("step: %5d  iteration: %d  thrs= %12.5e err=  %12.5e at element: %5d \n", 
                    tddft_steps, iter_scf,    thrs_dHmat,  err,         ij_err);  
            //err= -1.0e0 ;  
            iter_scf ++ ;
        } //---- end of  SCF/while loop 

        // RUNNING TIME-AVERAGED DENSITY for Ehrenfest forces
        //   trho(r) = (1/N_steps) * sum_n  rho(r, t_n)
        // accumulated here as a plain sum; rescaled by 1/ct.tddft_steps
        // after the outer time loop.  Used below in the TDDFT_CVE branch
        // as the input to Force(), instead of the final-snapshot rho.
        // Averaging damps the oscillatory part of the TD density and
        // produces smoother, more stable Ehrenfest forces.
        for(int i=0;i < trho.pbasis;i++) trho[i] += rho[i];

        RT2a = new RmgTimer("2-TDDFT: current and dipole");
        //  extract dipole from rho(Pn1)
        get_dipole(rho.data(), rhoc.data(), dipole_tot);
        /*  done with propagation,  save Pn1 ->  Pn0 */
        if(ct.tddft_energy)
        {
            static int header_once;
            Eterms[3] = etxc;
            tddft_energy(vh, rho, rhoc, Kptr, Mdim, Ndim, Eterms, eldyn_comm);
            if(tot_steps == 0)
            {
                if(pct.kstart == 0 && pct.gridpe == 0 && pct.spinpe == 0 && header_once == 0)
                {
                    Eterms_1step = Eterms;
                    fprintf(efi, "\n&& %16.8e  %16.8e  %16.8e  %16.8e %16.8e %16.8e at 1st TDDFT step", Eterms_1step[0],
                            Eterms_1step[1],Eterms_1step[2],Eterms_1step[3],Eterms_1step[4],Eterms_1step[5]);
                    fprintf(efi, "\n&&time  totalE     , EkinPseudo     , Vh     , Exc    , II   %s", eunits);
                    header_once++;
                }
                //for(int i = 0; i < 6; i++) Eterms_1step[i] = Eterms[i];
            }
            if(pct.kstart == 0 && pct.gridpe == 0 && pct.spinpe == 0)
            {
                fprintf(efi, "\n  %f  %16.8e %16.8e,%16.8e,%16.8e,%16.8e   ",
                        total_time, Eterms[0]*efactor, Eterms[1]*efactor, 
                        Eterms[2]*efactor, Eterms[3]*efactor, Eterms[4]*efactor);
            }
        }


        // ----------------------------------------------------------------
        // STEP 3 — ADVANCE: shift state variables for the next timestep
        //   P(t)     <- P(t+dt)     [Pn0  <- Pn1]
        //   H(t-dt)  <- H(t)        [Hmatrix_m1 <- Hmatrix_0]
        //   H(t)     <- H(t+dt)     [Hmatrix_0  <- Hmatrix_1]
        // ----------------------------------------------------------------
        for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++) {
            memcpy(Kptr[kpt]->Pn0_cpu, Kptr[kpt]->Pn1_cpu, n22 * sizeof(double));

            // save current  H0, H1 for the  next step extrapolatiion
            memcpy(Kptr[kpt]->Hmatrix_m1_cpu, Kptr[kpt]->Hmatrix_0_cpu, matrix_size);
            //dcopy(&n2, Hmatrix  , &ione, Hmatrix_1  , &ione);         // this update is already done right after scf loop

            memcpy(Kptr[kpt]->Hmatrix_0_cpu, Kptr[kpt]->Hmatrix_1_cpu, matrix_size);

            if(ct.tddft_mode == VECTOR_POT )
            {
                // TIME-DEPENDENT CURRENT J(t) — extracted after each propagation step
                // -------------------------------------------------------------------
                // The paramagnetic current density (in the orbital basis) is:
                //
                //   J_alpha(t) = Re[ Tr( P(t) * P^alpha ) ]
                //              = Re[ sum_ij  P_ij(t) * P^alpha_ij ]
                //
                // where:
                //   P(t)     = Pn0  (density matrix after advancing, stored as P_ij(t+dt))
                //   P^alpha  = Pxmatrix/Pymatrix/Pzmatrix (precomputed total current operator)
                //
                // The zdotc computes conj(P_ij) * P^alpha_ij, summed over all i,j.
                // Multiplied by kweight and summed over k-points for the BZ average:
                //
                //   J_alpha(t) = sum_k  w_k * Re[ Tr_k( P_k(t) * P^alpha_k ) ]
                //
                // This is the observable written to _current.dat at each step.
                // Its Fourier transform J(omega) / E(omega) gives the optical conductivity.
                std::complex<double> tem_x = rmg_zdotc(&n2, (std::complex<double> *)Kptr[kpt]->Pn0_cpu, &ione, (std::complex<double> *)Kptr[kpt]->Pxmatrix_cpu, &ione);
                current[0] += std::real(tem_x) * Kptr[kpt]->kp.kweight;
                std::complex<double> tem_y = rmg_zdotc(&n2, (std::complex<double> *)Kptr[kpt]->Pn0_cpu, &ione, (std::complex<double> *)Kptr[kpt]->Pymatrix_cpu, &ione);
                current[1] += std::real(tem_y) * Kptr[kpt]->kp.kweight;
                std::complex<double> tem_z = rmg_zdotc(&n2, (std::complex<double> *)Kptr[kpt]->Pn0_cpu, &ione, (std::complex<double> *)Kptr[kpt]->Pzmatrix_cpu, &ione);
                current[2] += std::real(tem_z) * Kptr[kpt]->kp.kweight;
            }
        }

        MPI_Allreduce(MPI_IN_PLACE, current, 3, MPI_DOUBLE, MPI_SUM, pct.kpsub_comm);
        MPI_Allreduce(MPI_IN_PLACE, current, 3, MPI_DOUBLE, MPI_SUM, eldyn_comm);
        Rmg_Symm->symm_vec(current);

        if(ct.BerryPhase && ct.tddft_mode == VECTOR_POT)
        {
            tot_bp_pol = 0.0;
            for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++) {
                std::complex<double> tem_x = rmg_zdotc(&n2, (std::complex<double> *)Kptr[kpt]->Pn0_cpu, &ione, (std::complex<double> *)Kptr[kpt]->BP_Xml, &ione);
                tot_bp_pol += std::real(tem_x) * Kptr[kpt]->kp.kweight;
            }
            MPI_Allreduce(MPI_IN_PLACE, &tot_bp_pol, 1, MPI_DOUBLE, MPI_SUM, pct.kpsub_comm);
            MPI_Allreduce(MPI_IN_PLACE, &tot_bp_pol, 1, MPI_DOUBLE, MPI_SUM, eldyn_comm);
            //Rmg_BP->CalcBP_tddft(Kptr, tot_bp_pol, matrix_glob, *Sp);
        }

        if(pct.kstart == 0 && pct.gridpe == 0)
        {
            if(ct.tddft_mode == VECTOR_POT )
            {
                fprintf(current_fi, "\n  %f  %18.10e  %18.10e  %18.10e ",
                        tot_steps*time_step, current[0], current[1], current[2]);
                if(ct.BerryPhase) fprintf(dbp_fi, "\n  %f  %18.10e  %18.10e  %18.10e ",
                        tot_steps*time_step, tot_bp_pol, 0.0,0.0);
            }
            else
            {
                fprintf(dfi, "\n  %f  %18.10e  %18.10e  %18.10e ",
                        tot_steps*time_step, dipole_tot[0], dipole_tot[1], dipole_tot[2]);
            }
        }

        delete RT2a;

        if((tddft_steps +1) % ct.checkpoint == 0)
        {   
            RT2a = new RmgTimer("2-TDDFT: Write");

            rmg::sync_device();
            for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++)
            {
                int kpt_glob = kpt + pct.kstart;

                std::string ofile = std::format("{}_spin{}_kpt{}_gridpe{}", 
                        ct.outfile_tddft, pct.spinpe, kpt_glob, pct.gridpe);
                WriteData_rmgtddft(ofile.c_str(), vh.data(), vxc.data(), vh_dipole.data(), (double *)Kptr[kpt]->Pn0_cpu, (double *)Kptr[kpt]->Hmatrix_cpu, 
                        (double *)Kptr[kpt]->Hmatrix_m1_cpu, (double *)Kptr[kpt]->Hmatrix_0_cpu, tot_steps+1, n2, n2_C, numst);
            }

            if(pct.kstart == 0 && pct.gridpe == 0)
            {
                if(ct.tddft_mode == VECTOR_POT )
                    fflush(current_fi);
                else
                {
                    fflush(dfi);
                    fflush(efi);
                }
                if(ct.BerryPhase)
                    fflush(dbp_fi);
            }
            delete RT2a;
        }

        total_time += time_step;
    } // end tddft md loop

    double rscale = 1.0 / (double)ct.tddft_steps;
    for(int i=0;i < trho.pbasis;i++) trho[i] *= rscale;
    trho.get_oppo();

    // ========================================================================
    // EHRENFEST FORCE UPDATE  (forceflag == TDDFT_CVE)
    //
    // After all TDDFT propagation steps for this MD slice are done, compute
    // the nuclear forces that will drive the next MD step.  This is the
    // "E" half of the C/V-E (Configuration / Velocity-Verlet — Ehrenfest)
    // coupling between the TD electrons and the classical nuclei.
    //
    // ALGORITHM
    // ---------
    //   1. RotateForces():    shift old per-ion force snapshots back by one
    //                         slot so the Beeman/V-V integrator has access
    //                         to F(t-dt), F(t-2dt), ... after this update.
    //
    //   2. gather_rho_matrix: collect the distributed density matrix Pn0 =
    //                         P(t_final, after the SCF loop) into a global
    //                         N_states x N_states matrix on each rank.
    //
    //   3. save_sint() + occupation swap + rmg::rotate_sint:
    //                         the ground-state sint = <beta|phi_gs> needs to
    //                         be rotated into the TD basis defined by the
    //                         current P(t).  rmg::rotate_sint multiplies sint
    //                         by the density matrix; occupations are temporarily
    //                         set to 1.0 because the per-orbital weights are
    //                         already encoded in P_ij.  Slot [3] holds the
    //                         original ground-state occupations for restore.
    //
    //   4. Force(trho, ...):  compute nuclear forces using the TIME-AVERAGED
    //                         density trho, not the final-snapshot density.
    //                         Averaging suppresses fast electronic oscillations
    //                         in the force and yields more stable trajectories.
    //
    //   5. restore_sint() + occupation restore + get_ddd:
    //                         undo the temporary modifications so subsequent
    //                         operations see the standard ground-state sint
    //                         and occupations.  get_ddd refreshes the D_ij
    //                         dual-projector coefficients used by the NL-PP.
    //
    // CAVEAT: as of this writing the nonlocal-PP contribution to the Ehrenfest
    // force is still under development (see develop-Ehrenfest branch).  The
    // analogue in the current operator is CurrentNlpp.cpp.
    // ========================================================================
    /*When running MD, force pointers need to be rotated before calculating new forces */
    if(ct.forceflag == TDDFT_CVE)
    {
        ct.fpt[0] = 0;
        ct.fpt[1] = 1;
        ct.fpt[2] = 2;
        ct.fpt[3] = 3;
        ct.sqrt_interpolation = false;

        for (size_t ion = 0, i_end = Atoms.size(); ion < i_end; ++ion)
        {
            Atoms[ion].RotateForces();
        }

        if(ct.internal_pseudo_type != ALL_ELECTRON)
        {
            rmg::hvector<OrbitalType> rho_matrix_global(ct.num_states*ct.num_states); 
            for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++)
            {
                gather_rho_matrix(rho_matrix_global.data(), (MatrixType *)Kptr[kpt]->Pn0_cpu);
                Kptr[kpt]->save_sint();
                for(int is=0;is < ct.num_states;is++)
                {
                    Kptr[kpt]->Kstates[is].occupation[3] = Kptr[kpt]->Kstates[is].occupation[0];
                    Kptr[kpt]->Kstates[is].occupation[0] = 1.0;
                }
                rmg::rotate_sint(Kptr[kpt], Kptr[kpt]->newsint_local, rho_matrix_global.data());
            }
        }
        Force (trho.up.data(), trho.dw.data(), rhoc.data(), vh.data(), vh.data(), vxc.data(), vxc.data(), vnuc.data(), Kptr);
        if(ct.internal_pseudo_type != ALL_ELECTRON)
        {
            for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++)
            {
                Kptr[kpt]->restore_sint();
                for(int is=0;is < ct.num_states;is++)
                    Kptr[kpt]->Kstates[is].occupation[0] = Kptr[kpt]->Kstates[is].occupation[3];
            }
        }
        get_ddd (vtot.data(), vxc.data(), true);

    }

}

template <typename OrbitalType, typename MatrixType>
rmg::tddft<OrbitalType, MatrixType>::~tddft(void)
{
    RmgTimer *RT2a;
#if CUDA_ENABLED || HIP_ENABLED
    rmg_device_pool->free(psi_dev_pool);
    rmg_device_pool->free(Pn1);
    rmg_device_pool->free(Pn0);
    rmg_device_pool->free(Hmatrix_1);
    rmg_device_pool->free(Hmatrix_0);
    rmg_device_pool->free(Hmatrix_m1);
    rmg_device_pool->free(Hmatrix);
#endif  
    if(pct.kstart == 0 && pct.gridpe == 0)
    {
        if(ct.tddft_mode == VECTOR_POT )
            fclose(current_fi);
        else
        {
            fclose(dfi);
        }
        if(ct.BerryPhase)
            fclose(dbp_fi);
    }
    if(ct.tddft_energy && pct.kstart == 0 && pct.gridpe == 0 && pct.spinpe == 0)
    {
        fclose(efi);
    }

    RT2a = new RmgTimer("2-TDDFT: Write");
    for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++)
    {
        int kpt_glob = kpt + pct.kstart;

        std::string ofile = std::format("{}_spin{}_kpt{}_gridpe{}",
                ct.outfile_tddft, pct.spinpe, kpt_glob, pct.gridpe);
        WriteData_rmgtddft(ofile.c_str(), vh.data(), vxc.data(), vh_dipole.data(), (double *)Kptr[kpt]->Pn0_cpu, (double *)Kptr[kpt]->Hmatrix_cpu,
                (double *)Kptr[kpt]->Hmatrix_m1_cpu, (double *)Kptr[kpt]->Hmatrix_0_cpu, tot_steps+1, n2, n2_C, numst);
    }
    delete RT2a;

    for(int kpt = 0; kpt < ct.num_kpts_pe; kpt++)
    {
        RmgFreeHost(Kptr[kpt]->Hmatrix_cpu);
        RmgFreeHost(Kptr[kpt]->Pn0_cpu);
        RmgFreeHost(Kptr[kpt]->Pn1_cpu);
        RmgFreeHost(Kptr[kpt]->Hmatrix_m1_cpu);
        RmgFreeHost(Kptr[kpt]->Hmatrix_1_cpu);
        RmgFreeHost(Kptr[kpt]->Hmatrix_0_cpu);
        if(ct.tddft_mode == VECTOR_POT)
        {
            RmgFreeHost(Kptr[kpt]->Pxmatrix_cpu);
            RmgFreeHost(Kptr[kpt]->Pymatrix_cpu);
            RmgFreeHost(Kptr[kpt]->Pzmatrix_cpu);

        }
        else
        {
            RmgFreeHost(Kptr[kpt]->Akick_cpu);
        }
    }

    if(Sp) delete Sp;
}


template <typename OrbitalType, typename MatrixType>
void rmg::tddft<OrbitalType, MatrixType>::tstconv(double *C,int *p_M, double *p_thrs,int *p_ierr, double *p_err, bool *p_tconv, MPI_Comm comm) 
{
    int     M     = *(p_M)    ;  //  [in]  :  total  size of matrix (2*Nbasis*Nbasis)
    double  thrs  = *(p_thrs) ;  //  [in]  :  convergence threshold
    double  err               ;  //  [out] :   error= abs of max element in the matrix
    int    ierr   =   0       ;  //  [out] :   location of err in matrix/vector
    bool   tconv  =  false    ;  //  [out] :   if converged ?  true or false?

    rmg::sync_device();
    if(!ct.tddft_gpu)
    { 
        err = abs(C[0]); 
        for (int i=0; i <M ;i++) {
            double err_tmp = abs(C[i]) ; 
            if (err_tmp > err) {
                err  = err_tmp ;
                ierr = i       ;
            }
        }
    }
    else
    {
#if CUDA_ENABLED || HIP_ENABLED
        int idx;
#if HIP_ENABLED
        hipblasIdamax(ct.gpublas_handle, M, C, 1, &idx);
#endif
#if CUDA_ENABLED
        cublasIdamax(ct.gpublas_handle, M, C, 1, &idx);
#endif
        idx -=1;    
        // hipblasIdamax return the index in fortran way, starting from 1
        gpuMemcpy(&err, &C[idx], sizeof(double), gpuMemcpyDeviceToHost);
        err = abs(err);
        ierr = idx;
#endif
    }

    rmg::sync_device();
    MPI_Allreduce(MPI_IN_PLACE, &err, 1, MPI_DOUBLE, MPI_MAX, comm);

    if (err < thrs)  tconv = true ;
    /*-- return values **/
    *(p_err )  =  err  ;
    *(p_ierr)  =  ierr ;
    *(p_tconv) =  tconv ;
}


template <typename OrbitalType, typename MatrixType>
void rmg::tddft<OrbitalType, MatrixType>::tstconv(float *C,int *p_M, double *p_thrs,int *p_ierr, double *p_err, bool *p_tconv, MPI_Comm comm) 
{
    int     M     = *(p_M)    ;  //  [in]  :  total  size of matrix (2*Nbasis*Nbasis)
    double  thrs  = *(p_thrs) ;  //  [in]  :  convergence threshold
    float  err               ;  //  [out] :   error= abs of max element in the matrix
    int    ierr   =   0       ;  //  [out] :   location of err in matrix/vector
    bool   tconv  =  false    ;  //  [out] :   if converged ?  true or false?


    rmg::sync_device();
#if CUDA_ENABLED || HIP_ENABLED
    int idx;
#if HIP_ENABLED
    hipblasIsamax(ct.gpublas_handle, M, C, 1, &idx);
#endif
#if CUDA_ENABLED
    cublasIsamax(ct.gpublas_handle, M, C, 1, &idx);
#endif
    idx -=1;    
    gpuMemcpy(&err, &C[idx], sizeof(float), gpuMemcpyDeviceToHost);
    err = abs(err);
    ierr = idx;
#else
    err = abs(C[0]); 
    for (int i=0; i <M ;i++) {
        double err_tmp = abs(C[i]) ; 
        if (err_tmp > err) {
            err  = err_tmp ;
            ierr = i       ;
        }
    }
#endif

    MPI_Allreduce(MPI_IN_PLACE, &err, 1, MPI_FLOAT, MPI_MAX, comm);

    if (err < thrs)  tconv = true ;
    /*-- return values **/
    *(p_err )  =  (double)err  ;
    *(p_ierr)  =  ierr ;
    *(p_tconv) =  tconv ;
}


template <typename OrbitalType, typename MatrixType>
void rmg::tddft<OrbitalType, MatrixType>::gather_rho_matrix(OrbitalType *rho_matrix_global, MatrixType *rho_matrix)
{   

    if(ct.tddft_tiledMM == 1)
    {
        double *rho_R = (double *)rho_matrix_global;
        std::complex<double> *rho_C = (std::complex<double> *)rho_matrix_global;
        size_t recvcount = numst * numst/pct.local_comm_npes * sizeof(OrbitalType)/sizeof(double);
        if(typeid(OrbitalType) == typeid(double))
        {
            for(int i = 0; i < numst * numst/pct.local_comm_npes; i++)
            {
                rho_R[pct.local_rank * numst * numst/pct.local_comm_npes + i] = std::real(rho_matrix[i]);
            }
        }
        else
        {
            for(int i = 0; i < numst * numst/pct.local_comm_npes; i++)
            {
                rho_C[pct.local_rank * numst * numst/pct.local_comm_npes + i] = rho_matrix[i];
            }
        }
        MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, 
                rho_matrix_global, recvcount, MPI_DOUBLE, pct.local_comm);       
    }
    else
    {
        int Mdim = Sp->GetDistMdim();
        int Ndim = Sp->GetDistNdim();
        std::vector<OrbitalType> rho_matrix_dist(Mdim * Ndim);
        double *rho_R = (double *)rho_matrix_dist.data();
        std::complex<double> *rho_C = (std::complex<double> *)rho_matrix_dist.data();
        if(typeid(OrbitalType) == typeid(double))
        {
            for(int i = 0; i < Mdim * Ndim; i++)
            {
                rho_R[i] = std::real(rho_matrix[i]);
            }
        }
        else
        {
            for(int i = 0; i < Mdim * Ndim; i++)
            {
                rho_C[i] = rho_matrix[i];
            }
        }

        for(int i = 0; i < numst * numst; i++) rho_matrix_global[i] = 0.0;
        Sp->GatherEigvectors(rho_matrix_global, rho_matrix_dist.data());
    }

}
