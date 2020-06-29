all: mrv.stl mrv.parts.stl

mrv.stl: mrv.scad
	openscad -o $@ -DB_SLOZENY=true $<
mrv.parts.stl: mrv.scad
	openscad -o $@ -DB_SLOZENY=false $<

clean:
	@rm -f *.stl

.PHONY: clean all
