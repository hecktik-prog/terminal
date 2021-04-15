#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define blocksize 128
#define ControlChars " \t\r\n\a"

//глобальные переменные
pid_t pid;  //идентификатор процесса
pid_t wpid; //идентификатор дочерного процесса

//метод, реализующий основной цикл обработки команд
void InteractingFunc();

//метод для считывания введённой строки
char* CommandReadingFunc();

//метод для парсинга строки
char** LineParsingFunc(char* line //введённая строка
                      );
//метод для запуска процессов
int ProcessLaunchingFunc(char** args //массив команд и аргументов
                        );

//метод для выполнения команды выхода
int exiting(char** args);

//метод для выполнения команды "помощь"
int help(char** args);

//метод для выполнения команды смены каталога
int cd(char** args);

//массив, содержащий встроенные в оболочку команды
char* commands[] = {"cd","help","exit"};

//массив указателей на функции, соответствующие встроенным командам
int (*CommandsFuncs[])(char**)= {&cd,&help,&exiting};

#define CommandsAmount (sizeof(commands)/sizeof(commands[0]))

int main(int argc,char** argv)
{

  //запуск основого цикла обработки команд
  InteractingFunc();
  
  return EXIT_SUCCESS;
}

/*	Реализация методов	*/

void InteractingFunc()
{
  char* line;     //указатель на считанную строку
  char** tokens;  //массив введённых команд, аргументов
  int choice;     //переменная для выхода из цикла
  char PathName[PATH_MAX]; //массив для помещения пути
  
  do
  {
    //получение пути к текущему каталогу
    getcwd(PathName,sizeof(PathName));
    printf("%s $",PathName);
    //считывание введённой строки
    line = CommandReadingFunc();
    //выделение команд, аргументов
    tokens = LineParsingFunc(line);
    //очищение выделенной памяти
    free(line);
    free(tokens);
    
  } while(choice);
}

char* CommandReadingFunc()
{
  char* line = NULL;       //указатель на считанную строку
  ssize_t sizeofblock = 0; //размер считываемого блока
  
  //чтение из потока stdin
  getline(&line,&sizeofblock,stdin);
  
  return line;
}

char** LineParsingFunc(char* line)
{
  int i=0; //счётчик
  int buf = blocksize;
  char* str;     //строка для выделенной лексемы
  char** parts;  //массив указателей на лексемы
  
  //выделение динамической памяти под массив указателей
  if ((parts=malloc(buf*sizeof(char*))) == NULL)
  {
    printf("LineParsingFunc(): Ошибка выделения памяти!");
    exit(EXIT_FAILURE);
  }
  
  //первое выделение
  str = strtok(line,ControlChars);
  while (str!= NULL)
  {
    parts[i] = str; //запись новой лексемы
    ++i;
    
    //если место в массиве кончилось, а лексемы остались
    //перераспределение массива
    if (i>=buf)
    {
      buf = buf + blocksize; //увеличение размера массива
      if ((parts=realloc(parts,buf*sizeof(char*))) == NULL)
      {
        printf("LineParsingFunc(): Ошибка перераспределения памяти!\n");
        exit(EXIT_FAILURE);
      }
    }
    //выделение следующей лексемы
    str=strtok(NULL,ControlChars);
  }
  //заверешение массива через NULL
  parts[i]=NULL;
  return parts;
}

int ProcessLaunchingFunc(char** args)
{
  int status; //состояние дочерного процесса
  
  //создание нового процесса
  pid = fork();
  
  //дочерний процесс
  if (pid == 0)
  {
    //запуск команды, заданной пользователем
    if (execvp(args[0],args)== -1)
    {
      perror("ProcessLaunchingFunc(): Ошибка!\n");
    }
    exit(EXIT_FAILURE);
  }
  //если выполнение fork() произошло с ошибкой
  else if (pid < 0)
  {
    perror("ProcessLaunchingFunc(): Ошибка при работе fork()!\n");
  }
  //родительский процесс
  else
  {
    do
    {
      //ожидание состояния процесса
      wpid = waitpid(pid,&status, WUNTRACED);
      //пока дочерний процесс не завершился
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  
  return 1;
}

int exiting(char** args)
{
  return 0;
}

int cd(char** args)
{
  //если пользователь не ввёл каталог, куда перемещаться
  if (args[1] == NULL)
  {
    printf("cd: Oшибка! Для команды ожидается аргумент\n");
  }
  else
  {
    //если не удалось сменить каталог
    if (chdir(args[1]) !=0)
    {
      perror("cd: Ошибка при попытке сменить каталог!\n");
    }
  }
  return 1;
}

int help(char** args)
{
  printf("Список команд:\n");
  for (int i = 0; i < CommandsAmount; ++i)
  {
    printf("%s\n",commands[i]);
  }
  return 1;
}
