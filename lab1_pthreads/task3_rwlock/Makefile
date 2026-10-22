CC     = gcc
CFLAGS = -O2 -Wall -Wextra -Wno-unused-result -pthread

all: ll_lib ll_my test_rwlock

ll_lib: pth_ll_rwl.c my_rand.c my_rand.h timer.h
	$(CC) $(CFLAGS) -o ll_lib pth_ll_rwl.c my_rand.c

ll_my: pth_ll_rwl.c my_rwlock.c my_rwlock.h my_rand.c my_rand.h timer.h
	$(CC) $(CFLAGS) -DUSE_MY_RWLOCK -o ll_my pth_ll_rwl.c my_rwlock.c my_rand.c

test_rwlock: test_rwlock.c my_rwlock.c my_rwlock.h my_rand.c my_rand.h
	$(CC) $(CFLAGS) -o test_rwlock test_rwlock.c my_rwlock.c my_rand.c

clean:
	rm -f ll_lib ll_my test_rwlock
