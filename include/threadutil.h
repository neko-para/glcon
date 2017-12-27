#ifndef _THREAD_UTIL_H_
#define _THREAD_UTIL_H_

#include <pthread.h>
#include <errno.h>

class Mutex {
private:
	pthread_mutex_t mutex;
	Mutex(const Mutex& ) {}
	Mutex& operator=(const Mutex& ) { return *this; }
public:
	Mutex() {
		pthread_mutex_init(&mutex, 0);
	}
	~Mutex() {
		pthread_mutex_destroy(&mutex);
	}
	void Lock() {
		pthread_mutex_lock(&mutex);
	}
	bool TryLock() {
		return EBUSY != pthread_mutex_trylock(&mutex);
	}
	void Unlock() {
		pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_t& Get() {
		return mutex;
	}
};

class MutexLock {
private:
	Mutex& mutex;
public:
	MutexLock(Mutex& m) : mutex(m) {
		mutex.Lock();
	}
	~MutexLock() {
		mutex.Unlock();
	}
};

class Cond {
private:
	pthread_cond_t cond;
	Mutex mutex;
	Cond(const Cond& ) {}
	Cond& operator=(const Cond& ) { return *this; }
public:
	Cond() {
		pthread_cond_init(&cond, 0);
	}
	~Cond() {
		pthread_cond_destroy(&cond);
	}
	void Lock() {
		mutex.Lock();
	}
	void Unlock() {
		mutex.Unlock();
	}
	void Signal() {
		pthread_cond_signal(&cond);
	}
	void SignalAll() {
		pthread_cond_broadcast(&cond);
	}
	void Wait() {
		pthread_cond_wait(&cond, &mutex.Get());
	}
	Mutex& GetLock() {
		return mutex;
	}
	pthread_cond_t& Get() {
		return cond;
	}
};

#endif