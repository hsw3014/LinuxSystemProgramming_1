#ifndef _EXTERN_
#define _EXTERN_

#include<string.h>

#define DEATH 5
#define WARNING 0.1
#define ERROR 0
#define BUFFER_SIZE 1024
#define MAX 105
#define SCORE_SIZE 8192

struct Questions{
	char q_name[MAX];
	double score;
	int type;
};

struct Students{
	char stu_num[MAX];
	double score[MAX];
	double sum;
};

extern char t;

extern char* scoreboard;
extern char* scoretable;
extern struct Students Students[MAX];
extern struct Questions Questions[MAX];
extern char opt_t_para[BUFFER_SIZE][MAX];//t옵션일때 받을 파라미터
extern char opt_c_para[BUFFER_SIZE][MAX];//c옵션일때 받을 파라미터
extern char opt_e_para[BUFFER_SIZE];         //e옵션일때 받을 파라미터

extern int opt_t_count;
extern int opt_c_count;

extern int fd_1, fd_2, fd_stdout;
extern int fd_student, fd_answer, fd_error;
extern int fd_stu_question;
extern char path_name[BUFFER_SIZE];
extern char fullname[BUFFER_SIZE];
extern char gccname[BUFFER_SIZE];
extern char check_c_array[BUFFER_SIZE];
extern char check_std_array[BUFFER_SIZE];
 
extern char result_scoreboard_name_array[MAX][BUFFER_SIZE];
extern char student_name_array[MAX][BUFFER_SIZE];
extern char answers_name_array[MAX][BUFFER_SIZE];

extern int student_size_array[MAX];
extern int answers_size_array[MAX];
extern int student_line_count;
extern int answer_line_count;
extern int result_student_count;
extern int result_answers_count;

extern char scoreboard_buf[SCORE_SIZE];
extern char std_dir[MAX];	//첫번째 학생디렉토리
extern char ans_dir[MAX];	//두번째 정답디렉토리

extern char imsy_buf[BUFFER_SIZE];

extern char path[BUFFER_SIZE];
extern char err_path[BUFFER_SIZE];
extern char err_name[BUFFER_SIZE];
#endif
