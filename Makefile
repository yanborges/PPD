CC=gcc

all: clean sequential openmp

sequential:
	$(CC) laplace_seq_iteracoes.c -o laplace_seq_iteracoes -Wall

openmp:
	# For Linux systems
	#$(CC) lp_omp.c -o lp_omp -fopenmp

	# For MacOS with clang
	/opt/homebrew/opt/llvm/bin/clang lp_omp.c -o lp_omp -fopenmp -L/opt/homebrew/opt/llvm/lib -Wall

test:
	./laplace_seq_iteracoes 1000
	./lp_omp 1000 1
	./lp_omp 1000 2
	./lp_omp 1000 4
	./lp_omp 1000 6
	./lp_omp 1000 8

check:
	diff grid_laplace_iter.txt grid_lp_omp.txt

clean:
	rm -f laplace_seq_iteracoes lp_omp


# /usr/bin/clang lp_omp.c -o lp_omp -fopenmp -L/usr/local/opt/llvm/lib 
# esse caminho do clang é o padrão do mac o qual nao reconhece omp, é preciso utilizar o do homebrew 
# é possível alterar o caminho do clang utilizando o json do workspace e não colocar o caminho aqui 