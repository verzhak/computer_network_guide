
OUTPUT_DIR = build
SRC = rep.tex
SRC_LAB = met.tex

debug: clean

	mkdir -p $(OUTPUT_DIR)
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC)

release: clean

	mkdir -p $(OUTPUT_DIR)
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC) 
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC) 
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC) 

debug_met: clean

	mkdir -p $(OUTPUT_DIR)
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC_LAB)

release_met: clean

	mkdir -p $(OUTPUT_DIR)
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC_LAB)
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC_LAB)
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC_LAB)

clean:

	rm -f $(OUTPUT_DIR)/*

distclean:

	rm -Rf $(OUTPUT_DIR)

mrproper:

	rm -Rf $(OUTPUT_DIR)

run: clean debug

	xpdf $(OUTPUT_DIR)/rep.pdf

release_run: clean release

	xpdf $(OUTPUT_DIR)/rep.pdf

run_met: clean debug_met

	xpdf $(OUTPUT_DIR)/met.pdf

release_run_met: clean release_met

	xpdf $(OUTPUT_DIR)/met.pdf

