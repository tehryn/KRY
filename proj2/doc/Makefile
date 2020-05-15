all: xmatej52.pdf

xmatej52.pdf: xmatej52.tex
	pdflatex xmatej52.tex
	bibtex xmatej52.aux
	pdflatex xmatej52.tex
	pdflatex xmatej52.tex

.PHONY: clean

clean:
	rm -f xmatej52.bbl xmatej52.log xmatej52.aux xmatej52.blg xmatej52.pdf xmatej52.toc