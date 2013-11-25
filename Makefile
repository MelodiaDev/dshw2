CXXFLAGS=-O3
FILES=apriori fp-tree optimized

all: $(FILES)

clean:
	rm -f $(FILES)
