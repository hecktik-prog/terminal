#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define blocksize 128
#define ControlChars " \t\r\n\a"

//метод, реализующий основной цикл обработки команд
void InteractingFunc();

//метод для считывания введённой строки
char* CommandReadingFunc();

//метод для парсинга строки
char** LineParsingFunc(char* line //введённая строка
                      );

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
