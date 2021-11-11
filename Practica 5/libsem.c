#include <pthread_utils.h>
#include <libsem.h>
#include <stdatomic.h>

atomic_int w = 0;
atomic_int sig = 0; 
// Macro que incluye el código de la instrucción máquina xchg
#define atomic_xchg(A,B) 	__asm__ __volatile__(	\
							"   lock xchg %1,%0 ;\n"	\
							: "=ir"  (A)			\
							: "m"  (B), "ir" (A)		\
							);

void initsem(SEMAPHORE *s,int val) {

	//Iniciamos el semaforo con el valor que se le da, así como la cola del semaforo.
	initqueue(&s->queue);
	s->count = val;
}

void waitsem(SEMAPHORE *s) {

	int xc = 1; //variable para exchange
	//atomic_xchg(xc, w);
	//Nos mantenemos ciclados mientras xc no sea 0

	while (xc != 0 ) {

		atomic_xchg(xc, w);
	}
	//Si el contador del semaforo es mayor a 0, seguimos decrementando
	if (s->count >0)
	{
		s->count --; 
	}
	//Si el contador es <= a 0, encolamos el hilo y lo bloqueamos
	else
	{
		s->count--;
		enqueue(&s->queue,pthread_self());
		block_thread();
	}
	
	w = 0; //Regresamos la variable w a 0
}

void signalsem(SEMAPHORE *s)
{
	int xc = 1;
	//atomic_xchg(xc, sig);
	
	while (xc != 0 )
	{
		atomic_xchg(xc, sig);
	}
	//Si la cola está vacia, sólo incrementamos el contador del semaforo.
	if (s->queue.exit == s->queue.entry)
	{
		s->count++;
		sig = 0;
		return;
	}
	//Si no está vacia y el contador es <= a 0, desencolamos y desbloqueamos el thread que esté en la cola
	if (s->count <=0)
	{
		s->count++;
		unblock_thread(dequeue(&s->queue));
	}
	sig=0; //Regresamos sig a 0

}

