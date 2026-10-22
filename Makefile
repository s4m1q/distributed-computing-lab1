TASK_DIRS := lab1_pthreads/task1_mandelbrot \
             lab1_pthreads/task2_nbody \
             lab1_pthreads/task3_rwlock

.PHONY: all check clean benchmark $(TASK_DIRS)

all: $(TASK_DIRS)

$(TASK_DIRS):
	$(MAKE) -C $@

check: all
	bash -n install_dependencies.sh
	bash -n run_benchmarks.sh
	bash -n lab1_pthreads/task1_mandelbrot/bench.sh
	bash -n lab1_pthreads/task2_nbody/bench.sh
	bash -n lab1_pthreads/task3_rwlock/bench.sh
	python3 -m py_compile lab1_pthreads/task1_mandelbrot/plot_mandelbrot.py \
		lab1_pthreads/task2_nbody/gen_bodies.py \
		lab1_pthreads/task2_nbody/plot_nbody.py
	./lab1_pthreads/task3_rwlock/test_rwlock 4 20000
	./lab1_pthreads/task3_rwlock/test_rwlock 18 10000

benchmark:
	bash run_benchmarks.sh

clean:
	$(MAKE) -C lab1_pthreads/task1_mandelbrot clean
	$(MAKE) -C lab1_pthreads/task2_nbody clean
	$(MAKE) -C lab1_pthreads/task3_rwlock clean
