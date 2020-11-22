CC = gcc

TARGET = des

run: build des/bin/${TARGET}
	@echo "Running..."
	./des/bin/${TARGET}

test: build
	@echo "Running all tests..."
	./des/test.py

build: des/src/${TARGET}.c des/src/${TARGET}.h
	@echo "Building ${TARGET}."
	${CC} des/src/${TARGET}.c -o des/bin/${TARGET}
	@echo "Done."

