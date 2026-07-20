
#all:  paper2_main.tex  sec_benchmarks.tex  sec_implementation.tex  sec_intro.tex  sec_theory.tex

#myfile=paper2_main
myfile=paper2_resolution
all:
	pdflatex ${myfile}.tex
	bibtex   ${myfile}
	pdflatex ${myfile}.tex 
