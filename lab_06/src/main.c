// Надо использовать неделимые операции:
// InterLockedIncrement, InterLockedDecrement.
// В программе должно быть 3 счетчика:
// ждущих писателей, ждущих читателей и активных читателей.
// Активный писатель м.б. только один и это логический тип.

#include <windows.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#define MINIMUM_READER_DELAY 100
#define MINIMUM_WRITER_DELAY 100
#define MAXIMUM_READER_DELAY 200
#define MAXIMUM_WRITER_DELAY 400

#define READERS_NUMBER 5
#define WRITERS_NUMBER 3

#define ITERATIONS_NUMBER 8

HANDLE can_read;
HANDLE can_write;
HANDLE mutex;

LONG waiting_writers_count = 0;
LONG waiting_readers_count = 0;
LONG active_readers_count = 0;

bool is_writer_active  = false;

HANDLE readerThreads[READERS_NUMBER];
HANDLE writerThreads[WRITERS_NUMBER];

int readersID[READERS_NUMBER];
int writersID[WRITERS_NUMBER];

int readersRand[READERS_NUMBER * ITERATIONS_NUMBER];
int writersRand[READERS_NUMBER * ITERATIONS_NUMBER];

int value = 0;

void StartRead()
{
	// ждущих читателей++
	InterlockedIncrement(&waiting_readers_count);

	if (is_writer_active  || WaitForSingleObject(can_write, 0) == WAIT_OBJECT_0)
	{
		WaitForSingleObject(can_read, INFINITE);
	}

	WaitForSingleObject(mutex, INFINITE);
	
	// ждущих читателей--
	InterlockedDecrement(&waiting_readers_count);
	
	// активных читателей++
	InterlockedIncrement(&active_readers_count);
	
	// Чтобы следующий читатель в очереди
	// Читателей смог начать чтение
	SetEvent(can_read);
	
	// ReleaseMutex() освобождает ранее захваченный мьютекс. 
	ReleaseMutex(mutex);
}

void StopRead()
{
	// активных читателей--
	InterlockedDecrement(&active_readers_count);

	if (!active_readers_count)
	{
		SetEvent(can_write);
	}
}

DWORD WINAPI Reader(CONST LPVOID param)
{
	int id = *(int *)param;
	int sleepTime;
	int begin = id * ITERATIONS_NUMBER;
	for (int i = 0; i < ITERATIONS_NUMBER; i++)
	{
		sleepTime = rand() % MAXIMUM_READER_DELAY + MINIMUM_READER_DELAY;
		Sleep(sleepTime);
		
		// CRITICAL START
		StartRead();
		printf("-> Reader id = %d; value = %d; sleep time = %d.\n", id, value, sleepTime);
		StopRead();
		// CRITICAL END
	}
}

void StartWrite()
{
	// ждущих писателей++
	InterlockedIncrement(&waiting_writers_count);

	if (is_writer_active  || active_readers_count > 0)
	{
		WaitForSingleObject(can_write, INFINITE);
	}

	// ждущих писателей--
	InterlockedDecrement(&waiting_writers_count);

	is_writer_active  = true;
	ResetEvent(can_write);
}

void StopWrite()
{
	is_writer_active  = false;
	
	if (waiting_readers_count)
	{
		SetEvent(can_read);
	}
	else
	{
		SetEvent(can_write);
	}		
}

DWORD WINAPI Writer(CONST LPVOID param)
{
	int id = *(int *)param;
	int sleepTime;
	int begin = id * ITERATIONS_NUMBER;
	
	for (int i = 0; i < ITERATIONS_NUMBER; i++)
	{
		sleepTime = rand() % MAXIMUM_WRITER_DELAY + MINIMUM_WRITER_DELAY;
		Sleep(sleepTime);

		// CRITICAL END
		StartWrite();
		++value;
		printf("<<< ----- Writer id = %d; value = %d; sleep time = %d.\n", id, value, sleepTime);
		StopWrite();
		// CRITICAL END	
	}
}

int init()
{
	// 2 == false значит мьютекс свободный.
	// 3 задает имя мьютекса, если нужно со
	// 4 здать именованный мьютекс. Если указывается NULL, то мьютекс не именуется. 
	mutex = CreateMutex(NULL, FALSE, NULL);
	if (mutex == NULL)
	{
		perror("CreateMutex\n");
		return -1;
	}

	// 2 == FALSE значит автоматический сброс.
	// 3 == FALSE значит, что объект не в сигнальном состоянии.
	// 4 == имя 
	if ((can_write = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
	{
		perror("CreateEvent (can_write)");
		return -1;
	}
	
	if ((can_read = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
	{
		perror("CreateEvent (can_read)");
		return -1;
	}
	
	return 0;
}

int CreateThreads()
{
	//DWORD id = 0;
	for (short i = 0; i < WRITERS_NUMBER; i++)
	{
		writersID[i] = i;
		if ((writerThreads[i] = CreateThread(NULL, 0, &Writer, writersID + i, 0, NULL)) == NULL)
		{
			perror("CreateThread (writer)");
			return -1;
		}
		// printf("Created writer thread id = %d\n", id);
	}
	
	for (short i = 0; i < READERS_NUMBER; i++)
	{
		readersID[i] = i;
		// Параметры слева направо:
		// NULL - Атрибуты защиты определены по умолчанию;
		// 0 - размер стека устанавливается по умолчанию;
		// Reader - определяет адрес функции потока, с которой следует начать выполнение потока;
		// readersID + i - указатель на переменную, которая передается в поток;
		//  0 - исполнение потока начинается немедленно;
		// Последний - адрес переменной типа DWORD, в которую функция возвращает идентификатор потока.
		if ((readerThreads[i] = CreateThread(NULL, 0, &Reader, readersID + i, 0, NULL)) == NULL)
		{
			perror("CreateThread (reader)");
			return -1;
		}
		// printf("Created reader thread id = %d\n", id);
	}

	return 0;
}

void Close()
{
	// Закрываем дескрипторы mutex, event и всех созданных потоков.
	for (int i = 0; i < READERS_NUMBER; i++)
	{
		CloseHandle(readerThreads[i]);
	}

	for (int i = 0; i < WRITERS_NUMBER; i++)
	{
		CloseHandle(writerThreads[i]);
	}

	CloseHandle(can_read);
	CloseHandle(can_write);
	CloseHandle(mutex);
}

int main(void)
{
	setbuf(stdout, NULL);
	srand(time(NULL));


	int rc = init();
	if (rc)
	{
		return -1;
	}

	rc = CreateThreads();
	if (rc)
	{
		return -1;
	}

	// READERS_NUMBER - кол-во инетерсующих нас объектов ядра.
	// readerThreads - указатель на массив описателей объектов ядра.
	// TRUE - функция не даст потоку возобновить свою работу, пока не освободятся все объекты.
	// INFINITE - указывает, сколько времени поток готов ждать освобождения объекта.
	WaitForMultipleObjects(WRITERS_NUMBER, writerThreads, TRUE, INFINITE);
	WaitForMultipleObjects(READERS_NUMBER, readerThreads, TRUE, INFINITE);


	Close();

	printf("\nFINISH\n");
	return 0;
}
