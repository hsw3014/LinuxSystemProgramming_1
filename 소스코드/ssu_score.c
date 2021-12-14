#include<stdio.h>	//전처리 전부다 이동시킬것
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include"checkstart.h"
#include"ssu_gettimeofday.h"

#define BUFFER_SIZE 1024
#define SCORE_SIZE 8192
#define MAX 105
#define O_MYFLAG O_RDWR | O_CREAT | O_TRUNC | O_APPEND

struct Questions{	//정답디렉토리 관련 정보들의 구조체
	char q_name[MAX];
	double score;
	int type;	//빈칸인지 채점인지 1 == txt, 2 == c
};

struct Students{
	char stu_num[MAX];
	double score[MAX];
};

struct Students Students[MAX];
struct Questions Questions[MAX];
char* scoreboard = "score_table.csv";	//점수테이블이름
char* scoretable = "score.csv";
int fd_score;			//scoreboard 파일디스크립터
double blank_score;		//빈칸문제 점수
double program_score;	//프로그램문제 점수
double users_score;		//사용자 직접설정 점수

char scoreboard_buf[SCORE_SIZE];
char std_dir[MAX];
char ans_dir[MAX];

char imsy_buf[BUFFER_SIZE];

char opt_t_para[BUFFER_SIZE][MAX] = {0};//t옵션일때 받을 파라미터
char opt_c_para[BUFFER_SIZE][MAX] = {0};//c옵션일때 받을 파라미터
char opt_e_para[BUFFER_SIZE] = {"default"};			//e옵션일때 받을 파라미터

int opt_t_count = 0;
int opt_c_count = 0;

char t;             //한글자씩 읽을때 저장용도
char path_name[BUFFER_SIZE] = {0};  //디렉토리간 이동을위한 저장버퍼
char fullname[BUFFER_SIZE] = {0};   //파일 읽어들일때 경로포함 이름
char gccname[BUFFER_SIZE] = {0};    //system하기위한 sprintf에 담을 버퍼
char check_c_array[BUFFER_SIZE] = {0};//.c파일 담기위한 버퍼
char check_std_array[BUFFER_SIZE] = {0};//std파일 이름 담기위한 버퍼

int student_size_array[MAX] = {0};	//임시 학생폴더의 한줄길이 저장 배열
int answers_size_array[MAX] = {0};	//임시 정답폴더의 한줄길이 저장 배열

int fd_1, fd_2, fd_stdout;	//파일 입출력, 정답의 fd_stdout을 만들기 위한 변수
int fd_student, fd_answer;	//임시 학생폴더, 정답폴더의 File Descripit
int fd_error;				//에러 파일디스크립터
int fd_stu_question;
int student_line_count = 0;
int answer_line_count = 0;
int result_student_count;	//최종 학생폴더 파일개수 - 1. (자기자신은 제외시켜야하므로)
int result_answers_count;	//최종 답지폴더 파일개수 - 1. (자기자신은 제외시켜야하므로)

int l_c = 0;	//정답리스트 배열증가용
int l_len = 0;	//정답리스트 한줄의 길이 체크변수
int l_list[MAX];	//정답리스트 길이 저장

char result_scoreboard_name_array[MAX][BUFFER_SIZE] = {0};	//최종 테이블에 넣을문제이름배열
char student_name_array[MAX][BUFFER_SIZE] = {0};	//학생이름 저장공간
char answers_name_array[MAX][BUFFER_SIZE] = {0};	//답 이름 저장공간

char path[BUFFER_SIZE];
char err_path[BUFFER_SIZE];
char err_name[BUFFER_SIZE];

int main(int argc, char* argv[])
{
	int o_e=0; int o_p=0; int o_t=0; int o_h=0; int o_c=0;

	char buf[BUFFER_SIZE];		//도움말 출력용 버퍼
	
	int opt_c_act = 0;
	char* opt_list[5] = {"-e","-p","-t","-h","-c"};
	char rem_opt[MAX] = {0};
	int skip_check = 0;	//옵션이면 스킵시키기위한 변수

	int tmp_fd;		//일시적인 fd. 닫기위해사용
	int opt;
	int i,j;
	int choice = 0;

	//-h옵션 거름
	gettimeofday(&begin_t, NULL);

	for(i=1; i<argc; i++){
		if(strcmp(argv[i],"-h") == 0){
			int helper;
			int h_length;
			off_t h_filesize;
			if((helper = open("option_h.txt", O_RDONLY)) < 0){
				fprintf(stderr, "open error for %s\n","option_h.txt");
				exit(1);
			}
			else{
				h_filesize = lseek(helper, 0, SEEK_END);
				lseek(helper, 0, SEEK_SET);
				h_length = read(helper, buf, h_filesize);
				write(1, buf, h_length);
				close(helper);
				exit(0);
			}
		}
	}

	if(argc <= 2){
		fprintf(stderr,"Can't execute with two parameter. Excepts -h option.\n");
		exit(1);
	}
	
	//DIR안주고 혹은 하나만주고 -c옵션사용시 -c옵션만 발생 
	if(strcmp(argv[1], "-c") == 0){
		//c옵션처리

		for(i=7; i<argc; i++){	//5개초과시에만 작동
			printf("Maximum Number of Argument Exceeded. :: %s\n",argv[i]);
		}

		int score_fd;
		if((score_fd = open(scoretable, O_RDONLY | O_APPEND | O_EXCL)) < 0){
			fprintf(stderr, "%s file not exists.", "score.scv");
			exit(1);
		}

		char* ptr;
		char* sum_ptr;
		read(score_fd, scoreboard_buf, SCORE_SIZE);		//파일전체를 읽어들임
		ptr = strtok(scoreboard_buf, "\n");		//가장 첫번째줄을 스킵함

		lseek(score_fd, 0, SEEK_SET);	//커서 처음으로이동
		int comma_count = 0;
		char rem_student_name[BUFFER_SIZE];	//학생이름들 기억 (한행에 하나씩)
		while(read(score_fd, &t, 1) > 0){
			if(t == ',')	//,개수만큼 카운트하여 sum까지 이동시킴
				comma_count++;
			if(t == '\n')
				break;
		}
		
		//c에 해당하는 학생이나오면 sum출력
		while((ptr = strtok(NULL, ",")) != NULL){		
			strcpy(rem_student_name, ptr);	//학생번호받음 20190001, 20190002...
			for(i=0; i<comma_count-1; i++){
				ptr = strtok(NULL, ",");
			}
			
			ptr = strtok(NULL, "\n");

			for(i=2; i<argc; i++){
				if(i >= 7)
					break;	//5개가 넘어가면 출력시키지 않음
				if(strcmp(rem_student_name, argv[i]) == 0){	//비교해서 같으면 출력
					double psum = atof(ptr);
					printf("%s's score : %.2lf\n",argv[i], psum);
					break;	//1번만 출력시킴. 더이상 검사하지않음. (중복제거)
				}
			}
		}
		exit(1);
	}
	else{
	//채점 전 전처리(파일이름 긁어오기 student.txt, answer.txt)
		if(argc >= 3){
			strcpy(std_dir,argv[1]);
			strcpy(ans_dir,argv[2]);
		}
		sprintf(path_name, "./%s",argv[1]);
	    if(chdir(path_name) == -1){
	        fprintf(stderr, "There is not exists %s\n directory.\n",path_name);
	        exit(1);
	    }
	    else{
	        if((fd_student = open(argv[1],O_MYFLAG, 0777)) < 0){
	            fprintf(stderr, "open error for %s\n",argv[1]);
	            exit(1);
	        }
	        else{
	            fd_1 = dup(1);
	            dup2(fd_student,1);
	            system("ls -v1");
	            dup2(fd_1,1);
	            lseek(fd_student, 0, SEEK_SET);
	            while(read(fd_student, &t, 1) > 0){
	                if(t == '\n'){
	                    student_line_count++;
	                }
	                else{
	                    student_size_array[student_line_count]++;
	                }
	            }

				lseek(fd_student, 0, SEEK_SET);
				//학생명단 배열화
				for(i=0; i<student_line_count-1; i++){
					read(fd_student, student_name_array[i], student_size_array[i]);
					lseek(fd_student, 1, SEEK_CUR);
				}
	        }
	        lseek(fd_student, 0, SEEK_SET);
    	}

		chdir("../");
    	sprintf(path_name, "./%s", argv[2]);
	
	    //answer.txt 생성
	    if(chdir(path_name) == -1){
	        fprintf(stderr, "There is not exists %s\n directory.\n",path_name);
	        exit(1);
	    }
	    else{
	        if((fd_answer = open(argv[2],O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0666)) < 0){
	            fprintf(stderr, "open error for %s\n",argv[2]);
	            exit(1);
	        }
    	    else{
    	        fd_1 = dup(1);
    	        dup2(fd_answer,1);
    	        system("ls -v1");
    	        dup2(fd_1,1);
  	
  	          	lseek(fd_answer, 0, SEEK_SET);
  	          	while(read(fd_answer, &t, 1)){
					 if(t == '\n'){
						 answer_line_count++;
   	            	 }
   	            	 else{
   	            	     answers_size_array[answer_line_count]++;
   	            	 }
				}
			}
			lseek(fd_answer, 0, SEEK_SET);
			//문제이름들 배열화
			for(i=0; i<answer_line_count-1; i++){
				char ans_path[MAX];
				read(fd_answer, answers_name_array[i], answers_size_array[i]);
				lseek(fd_answer, 1, SEEK_CUR);	//구분개행 스킵
				
				sprintf(ans_path,"./%s/%s.txt",answers_name_array[i], answers_name_array[i]);
				if((tmp_fd = open(ans_path, O_RDONLY)) < 0){	//txt파일이 없다면 채점파일
					sprintf(result_scoreboard_name_array[i],"%s.c",answers_name_array[i]);
					Questions[i].type = 2;
					strcpy(Questions[i].q_name, answers_name_array[i]);
					close(tmp_fd);
				}
				else{
					sprintf(result_scoreboard_name_array[i],"%s.txt",answers_name_array[i]);
					Questions[i].type = 1;
					strcpy(Questions[i].q_name, answers_name_array[i]);
					close(tmp_fd);
				}
				
			}
			lseek(fd_answer, 0, SEEK_SET);
		}
		chdir("../");
 		result_student_count = student_line_count - 1;//임시생성폴더는 제외
   		result_answers_count = answer_line_count - 1;//임시생성폴더는 제외
  		student_size_array[student_line_count] = 0;
  		answers_size_array[answer_line_count] = 0;
	}
	//점수테이블 파일 생성
	if((fd_score = open(scoreboard,O_RDWR | O_CREAT | O_EXCL, 0777)) > 0){
		printf("%s file doesn't exist in TRUEDIR!\n",scoreboard);
		printf("1. input blank question and program question's score. ex) 0.5 1 \n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1 \n");
		
		printf("select type >> ");
		scanf("%d", &choice);

		if(choice == 1){//빈칸, 프로그램채점점수 입력받음
			while(1){
				printf("Input value of blank question : ");
				scanf("%lf",&blank_score);
				if(blank_score > 0)
					break;
			}
			while(1){
				printf("Input value of program question : ");
				scanf("%lf",&program_score);
				if(program_score > 0)
					break;
			}
			for(i=0; i<result_answers_count; i++){
				if(Questions[i].type == 1)
					Questions[i].score = blank_score;
				else if(Questions[i].type == 2)
					Questions[i].score = program_score;
			}

		}
		else if(choice == 2){	//정답폴더내의 모든 정답들에 대해 점수를 매김
			for(i=0; i<result_answers_count; i++){
				printf("Input of %s.txt: ", answers_name_array[i]);
				scanf("%lf",&users_score);
				strcpy(Questions[i].q_name, answers_name_array[i]);
				Questions[i].score = users_score;
			}	
		}
		else{
			fprintf(stderr, "Input 1 or 2\n");
			sprintf(imsy_buf, "rm %s",scoreboard);
			system(imsy_buf);
			exit(1);
		}
		
		// 테이블에 입력
		for(i=0; i<result_answers_count; i++){
			if(Questions[i].type == 1){
				sprintf(imsy_buf, "%s.txt,%.2lf\n", answers_name_array[i], Questions[i].score);
				write(fd_score, imsy_buf, strlen(imsy_buf));
			}
			else if(Questions[i].type == 2){
				sprintf(imsy_buf, "%s.c,%.2lf\n", answers_name_array[i], Questions[i].score);
				write(fd_score, imsy_buf, strlen(imsy_buf));
			}
		}
		close(fd_score);
	}
	else{ //정답파일이 존재함
		fd_score = open(scoreboard, O_RDONLY);
		lseek(fd_score, 0, SEEK_SET);
		while(read(fd_score, &t, 1) > 0){
			if(t == '\n'){
				l_list[l_c] = l_len;
				l_len = 0;
				l_c++;
				continue;
			}
			else{
				l_len++;
			}
		}
		
		lseek(fd_score, 0, SEEK_SET);	//오프셋 처음으로
		if(l_c != result_answers_count){	//채점할 문제수와 테이블의 문제 수가 다르면 에러
			fprintf(stderr, "Table and ANS_DIR's questions are not matched!\n");
			close(fd_student);
			close(fd_answer);
			sprintf(gccname, "rm ./%s/%s", argv[1], argv[1]);
			system(gccname);
			sprintf(gccname, "rm ./%s/%s", argv[2], argv[2]);
			system(gccname);
			exit(1);
		}
		
		//표의 이름과 점수를 옮겨줌
		for(i=0; i<result_answers_count; i++){
			if(read(fd_score, imsy_buf, l_list[i]) < 0){
				fprintf(stderr, "read error for %d\n",i);
				exit(1);
			}
			else{
				if(l_list[i] == 0){//테이블안에 공백이있는 줄이있으면 에러
					fprintf(stderr, "There is a blank in table.\n");
					exit(1);
				}
				lseek(fd_score, 1, SEEK_CUR);
				char* ptr = strtok(imsy_buf, ",");	//문제이름 1-1.txt
				strcpy(Questions[i].q_name, ptr);
		
				ptr = strtok(NULL, ",");		//점수 0.2
				Questions[i].score = atof(ptr);
			}

			//유효한 문제인지 체크
			for(j=0; j<result_answers_count; j++){
				char* ptr = strtok(Questions[i].q_name, ".");
				if(strcmp(ptr, answers_name_array[j]) == 0)	//유효하면 나감
					break;
				if(j == (result_answers_count-1)){	//끝까지돌았으나 해당 문제없으면 에러
					fprintf(stderr,"There is no %s questions name.\n", Questions[i].q_name);
					exit(1);
				}
			}
		}
		close(fd_score);
	}
	
	//옵션별 세부정리
	for(i=3; i<argc; i++){
		//printf("rem : %s argv : %s\n",rem_opt, argv[i]);
		for(j=0; j<5; j++){
			if((strcmp(argv[i], opt_list[j]) == 0)){	//옵션이면 기억 후 스킵
				strcpy(rem_opt, opt_list[j]);
				skip_check = 1;
				break;
			}
		}
		if(!skip_check){
			if(strcmp(argv[i],"-") == 0){
				fprintf(stderr,"Error for Option. Not Valuabe\n");
				exit(1);
			}
			else if(strcmp(rem_opt,"-t") == 0){
				strcpy(opt_t_para[opt_t_count], argv[i]);
				opt_t_count++;
			}
			else if(strcmp(rem_opt,"-c") == 0){
				strcpy(opt_c_para[opt_c_count], argv[i]);
				opt_c_count++;
			}
		}
		else
			skip_check = 0;
	}

	//어떤 옵션이 부여되었는지 체크
	while((opt = getopt(argc, argv, "e:pthc")) != -1){
		switch(opt)
		{
			case 'e' : o_e++; sprintf(opt_e_para, "%s", optarg); break;
			case 'p' : o_p++; break;
			case 't' : o_t++; break;
			case 'h' : o_h++; break;
			case 'c' : o_c++; break;
			default : fprintf(stderr, "Error for Option. Not Valuable\n");
					  exit(1);;
		}
		if(o_e > 1 || o_p > 1 || o_t > 1 || o_h > 1 || o_c > 1){
			fprintf(stderr, "Used same option more twice.\n");
			exit(1);
		}
	}

	//초과 인자들 출력후 채점으로
	if(opt_t_count > 5 || opt_c_count > 5){
		for(i=5; i<opt_t_count; i++)
			printf("Maximum Number of Argument Exceeded. :: %s\n",opt_t_para[i]);
		for(i=5; i<opt_c_count; i++)
			printf("Maximum Number of ARgument Exceeded. :: %s\n",opt_c_para[i]);
	}
	act(o_e, o_p, o_t, o_h, o_c);
	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
}
