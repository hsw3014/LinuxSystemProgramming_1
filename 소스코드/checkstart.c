#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include"extern_var.h"
#include"checkstart.h"

int act(int o_e, int o_p, int o_t, int o_h, int o_c)
{
	double result_score;
	double minus_score;
	int ft_error;
	int t_count = 0;
	int temp_fd;	
	int i,j,k;
	int e_checker = 0;
	int copy_fd;

	//채점 전 정답파일 내 stdout파일 생성
	for(j=0; j<result_answers_count; j++){
		char ansdir_name[MAX] = {0};	//매번 초기화
		read(fd_answer, ansdir_name, answers_size_array[j]);//채점대상 선택(1-1,...)
		lseek(fd_answer, 1, SEEK_CUR);
		sprintf(path_name, "./%s/%s", ans_dir, ansdir_name);//./ANS_DIR/1-1

		sprintf(check_c_array, "%s/%s.c",path_name,ansdir_name);//열고자 하는 파일
		sprintf(fullname, "%s/%s", path_name, ansdir_name);	//최종파일(확장자제외)

		if((temp_fd = open(check_c_array, O_RDONLY)) < 0){	//프로그램 문제아니면 스킵
			close(temp_fd);
			continue;
		}
		else{
			//.c파일이 있으므로 컴파일시킴
			sprintf(check_std_array, "%s.stdout", fullname);
			fd_stdout = open(check_std_array, O_RDWR | O_CREAT | O_TRUNC, 0777);

			sprintf(gccname,"gcc -o %s.exe %s.c -lpthread", fullname, fullname);
			system(gccname);

			fd_1 = dup(1);
			dup2(fd_stdout, 1);
			close(fd_stdout);
			sprintf(gccname,"./%s.exe", fullname);
			system(gccname);	//stdout파일 생성
			dup2(fd_1, 1);
			close(fd_1);
			close(temp_fd);
		}
	}
	
	printf("grading students's test papers..\n");

	//모든에러 담을 디렉토리 생성
	//이미 파일이 존재하면 스킵
	
	if(access(opt_e_para, F_OK) < 0){
		sprintf(gccname,"mkdir %s",opt_e_para);
		system(gccname);	//e옵션이 있으면 해당이름으로, 없으면 default
	}

	//학생번호 선택후 채점시작
	for(i=0; i<result_student_count; i++){
		e_checker = 0;
		read(fd_student, (char*)&(Students[i].stu_num), student_size_array[i]);	//20190001
		lseek(fd_student, 1, SEEK_CUR);	//개행스킵

		//학생별 에러 디렉토리 생성
		//이미 에러 디렉토리가 존재하면 스킵함
		sprintf(imsy_buf, "./%s/%s", opt_e_para, Students[i].stu_num);
		if(access(imsy_buf, F_OK) < 0){
			sprintf(gccname, "mkdir ./%s/%s", opt_e_para, Students[i].stu_num);
			system(gccname);
		}

		//문제별 채점시작
		for(j=0; j<result_answers_count; j++){
			if(Questions[j].type == 1){	//빈칸문제임
				Students[i].score[j] = blankcheck(i,j,Questions[j].score);
				Students[i].sum+=Students[i].score[j];
			}
			else if(Questions[j].type == 2){	//채점문제임
				int t_checker = 0;
				for(k=0; k<5; k++){	//문제가 t옵션에 해당하는지 체크
					if(strcmp(answers_name_array[j], opt_t_para[k]) == 0)
						t_checker = 1;
				}

				char path[BUFFER_SIZE];
				char err_file[BUFFER_SIZE];
				char err_name[BUFFER_SIZE];
				char err_path[BUFFER_SIZE];
				sprintf(path, "./%s/%s/%s", std_dir, Students[i].stu_num, Questions[j].q_name);
				sprintf(err_file, "./%s/%s/%s_error.txt", opt_e_para, Students[i].stu_num, answers_name_array[j]);
				sprintf(err_path, "./%s/%s", opt_e_para, Students[i].stu_num);
				
				//에러파일 생성
				if((fd_error = open(err_file, O_RDWR | O_CREAT | O_TRUNC, 0777)) < 0){
					fprintf(stderr, "open error for %s\n",err_file);
					exit(1);
				}
				ft_error = dup(fd_error);
				fd_2 = dup(2);
				dup2(fd_error, 2);
				close(fd_error);

				// 컴파일시작
				if(t_checker == 1){	//-lpthread 옵션
					sprintf(gccname, "gcc -o %s.stdexe %s.c -lpthread", path, path);
					system(gccname);
				}
				else{
					//gcc
					sprintf(gccname, "gcc -o %s.stdexe %s.c", path, path);
					system(gccname);
				}
				dup2(fd_2, 2);
				close(fd_2);

				//에러파일 관리
				if(lseek(ft_error, 0, SEEK_END) == 0){//에러담는파일이 비어있음 = 에러없음
					sprintf(gccname, "rm %s", err_file);
					system(gccname);
					close(ft_error);
				}

				else{	//뭔가 에러가 발견됨 (warning체크해야함)
					e_checker = 1;
					int is_error;
					int warning_count = 0;
					int w_size = lseek(ft_error, 0, SEEK_END);
					char w_buf[w_size+5];
					lseek(ft_error, 0, SEEK_SET);
					read(ft_error, w_buf, w_size);
					char* ptr = strtok(w_buf, " ");
					while(ptr != NULL){		//warning 개수세기 시작
						if(strcmp(ptr,"warning:") == 0)
							warning_count++;
						if(strcmp(ptr,"error:") == 0){
							is_error = 1;
						}
						ptr = strtok(NULL, " ");
					}

					if(is_error){		//에러면 실행불가능
							Students[i].score[j] = ERROR;
							continue;
					}
					else{
						minus_score = WARNING * warning_count;
					}
				}
				
				//여기에 도달했다는 것은 실행이 가능하다는것을 의미한다.
				sprintf(imsy_buf, "%s.stdout", path);
				fd_1 = dup(1);
				fd_stu_question = open(imsy_buf, O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0744);
				copy_fd = dup(fd_stu_question);
				dup2(fd_stu_question, 1);
				close(fd_stu_question);
				
				int count = 0;
				int kill;
				int five_second = 0;
				char p_space[BUFFER_SIZE];
				sprintf(imsy_buf, "%s.stdexe &", path);	//백그라운드로 돌림
				system(imsy_buf);
				while(1){
					int p_1 = dup(1);
					int pid_fd;
					int c_pid_fd;
					count++;
					if(count == DEATH+1){	//5초 킬
						five_second = 1;
						sprintf(p_space, "kill -9 %d",kill);
						system(p_space);
						close(c_pid_fd);
						break;
					}
					if((pid_fd = open("pid_list.txt", O_RDWR | O_CREAT | O_TRUNC, 0644)) < 0){
						fprintf(stderr, "open error for %s\n","pid_list.txt");
						exit(1);
					}

					c_pid_fd = dup(pid_fd);
					dup2(pid_fd, 1);
					close(pid_fd);

					sprintf(imsy_buf, "ps -o pid,cmd --sort=-pid | grep %s.stdexe",path);
					system(imsy_buf);

					dup2(p_1, 1);
					close(p_1);
					
					lseek(c_pid_fd, 0, SEEK_SET);
					int enter_count = 0;
					while(read(c_pid_fd, &t, 1) > 0){
						if(t == '\n'){
							enter_count++;
						}
						if(enter_count == 3)
							break;
					}	//엔터개수 카운트

					int end_count = 0;

					lseek(c_pid_fd, 0, SEEK_SET);
					while(read(c_pid_fd, &t, 1) > 0){
						if(t == '\n'){
							end_count++;
						}
						if(end_count == enter_count-1)
							break;
					}	//3번째 줄에 해당하는쪽의 pid
					
					if(enter_count >= 3){
						read(c_pid_fd, p_space, BUFFER_SIZE);
						char* ptr = strtok(p_space, " ");
						if(count == 1)
							kill = atoi(ptr);
					}
					else{
						break;
					}

					sleep(1);
				}
				dup2(fd_1, 1);
				close(fd_1);

				
				//채점
				if(five_second == 1){
					Students[i].score[j] = 0;
					continue;
				}

				sprintf(imsy_buf, "%s/%s/%s.stdout", ans_dir, answers_name_array[j], answers_name_array[j]);
				fd_stdout = open(imsy_buf, O_RDONLY);
				
				int max;
				int l_size = lseek(copy_fd, 0, SEEK_END);
				int r_size = lseek(fd_stdout, 0, SEEK_END);
				max = l_size >= r_size ? l_size : r_size;
				char versus_buf1[max+5];
				char versus_buf2[max+5];
				int buf1_count = 0;
				int buf2_count = 0;

				lseek(copy_fd, 0, SEEK_SET);
				lseek(fd_stdout, 0, SEEK_SET);
					
				while(read(copy_fd, &t, 1) > 0){	//학생
					if('A' <= t && t <= 'Z')
						t-=32;		//대문자는 소문자로
					if(t == ' ')
						continue;	//공백은 무시
					versus_buf1[buf1_count] = t;	//1글자씩채움
					buf1_count++;
				}
				versus_buf1[buf1_count] = 0;
					
				while(read(fd_stdout, &t, 1) > 0){	//정답
					if('A' <= t && t <= 'Z')
						t-=32;		//대문자는 소문자로
					if(t == ' ')
						continue;	//공백은 무시
					versus_buf2[buf2_count] = t;	//1글자씩채움
					buf2_count++;
				}
				versus_buf2[buf2_count] = 0;

				if(strcmp(versus_buf1, versus_buf2) == 0){	//다 맞으면 정답, 점수부여
					Students[i].score[j] = Questions[j].score - minus_score;
					if(Students[i].score[j] < 0)
						Students[i].score[j] = 0;
				}
				else	//틀리면 0점
					Students[i].score[j] = 0;

				Students[i].sum+= Students[i].score[j];	//합 상승
				close(copy_fd);
				close(fd_stdout);
			}
			else{
				fprintf(stderr, "error for type\n");
				exit(1);
			}
		}

		if(e_checker == 0){	//에러가 없음 -> error폴더의 학생폴더 삭제
			sprintf(gccname, "rm -rf %s/%s", opt_e_para, Students[i].stu_num);
			system(gccname);
		}

		printf("%s is finished.. ",Students[i].stu_num);
		if(o_p)	//-p옵션을 통한 점수출력
			printf("score : %.2lf\n", Students[i].sum);
		else
			printf("\n");

	}

	if(o_p){	//-p옵션을 통한 평균출력
		double avr;
		for(i=0; i<result_student_count; i++)
			avr+=Students[i].sum;
		printf("Total Average : %.2lf\n", avr/result_student_count);
	}

	if(o_c){	//-c옵션을통한 점수출력
		for(i=0; i<result_student_count; i++){
			for(j=0; j<opt_c_count; j++){
				if(j >= 5)
					break;	//5개가 넘어가면 출력시키지 않음
				if(strcmp(Students[i].stu_num, opt_c_para[j]) == 0){
					printf("%s's score : %.2lf\n", Students[i].stu_num, Students[i].sum);
					break;	//1번만 출력시킴. 더이상 검사하지않음. (중복제거)
				}
			}
		}
	}

	if(o_e == 0)	//저장공간 삭제
		system("rm -rf default");

	//채점결과테이블 생성
	int result_scoreboard_fd;
	if((result_scoreboard_fd = open("score.csv", O_RDWR | O_CREAT | O_TRUNC, 0777)) < 0){
		fprintf(stderr, "open error for %s\n","score.csv\n");
		exit(1);
	}

	char result_scoreboard_temp[BUFFER_SIZE] = {0};

	for(i=-1; i<result_answers_count+1; i++){
		if(i == -1){
			sprintf(result_scoreboard_temp, ",");
			write(result_scoreboard_fd, result_scoreboard_temp, strlen(result_scoreboard_temp));
		}
		else if(i == result_answers_count){
			sprintf(result_scoreboard_temp, "sum\n");
			write(result_scoreboard_fd, result_scoreboard_temp, strlen(result_scoreboard_temp));
		}
		else{
			sprintf(result_scoreboard_temp, "%s,",result_scoreboard_name_array[i]);
			write(result_scoreboard_fd, result_scoreboard_temp, strlen(result_scoreboard_temp));
		}
	}

	for(i=0; i<result_student_count; i++){
		sprintf(result_scoreboard_temp, "%s,", student_name_array[i]);
		write(result_scoreboard_fd, result_scoreboard_temp, strlen(result_scoreboard_temp));
		for(j=0; j<result_answers_count+1; j++){
			if(j == result_answers_count){
				sprintf(result_scoreboard_temp, "%.2lf\n", Students[i].sum);
				write(result_scoreboard_fd, result_scoreboard_temp, strlen(result_scoreboard_temp));
			}
			else{
				sprintf(result_scoreboard_temp, "%.2lf,", Students[i].score[j]);
				write(result_scoreboard_fd, result_scoreboard_temp, strlen(result_scoreboard_temp));
			}
		}
	}
	
	close(result_scoreboard_fd);
	//파일테이블을 만들고 close



	//임시파일들 지움
	sprintf(gccname, "rm ./%s/%s", std_dir, std_dir);
	system(gccname);
	sprintf(gccname, "rm ./%s/%s", ans_dir, ans_dir);
	system(gccname);
	system("rm pid_list.txt");
}
