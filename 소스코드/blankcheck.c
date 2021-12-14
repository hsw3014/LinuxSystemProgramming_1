#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include"extern_var.h"

#ifndef IS_CHARACTER
#define IS_CHARACTER (t == '_' || ('0' <= t && t <= '9') || ('A' <= t && t <= 'Z') || ('a' <= t && t <= 'z'))
#define TWO_OPERATOR_LENGTH 11
#define SWAP_LENGTH 10
#define CHANGE_LENGTH 4

#endif
//교환법칙에 영향을 미치는 오퍼레이터 
char swap_operator_list[20][MAX] = {
	">", ">=", "<", "<=", "&&", "||", "&", "|", "^", "=="};
char two_operator_list[20][MAX] = {
	"==", ">=", "<=", "||", "&&", "->", "!=", "|=", "&=", "+=", "*="};
char change_operator_list[20][MAX] = {
	">", ">=", "<", "<="};

char big_token[MAX][BUFFER_SIZE] = {0};	//정답리스트토큰담음
char ans_big_token[MAX][BUFFER_SIZE] = {0};

char all_token[MAX][BUFFER_SIZE] = {0};	//모든 토큰을 가진 버퍼
char ans_all_token[MAX][BUFFER_SIZE] = {0};	//답지에서 모든 토큰을 가진버퍼, 위 버퍼와 비교할것

char var_token[MAX][MAX] = {0};			//변수명만 담기는 최소의 토큰
char opr_token[MAX][10] = {0};			//연산자만 담기는 최소의 토큰

char ans_opr_token[MAX][10] = {0};		//답지의 연산자 담는 토큰

char token[BUFFER_SIZE] = {0};

char path[BUFFER_SIZE] = {0};

int std_blank_fd = {0};
int ans_blank_fd = {0};

int count;

int big_count;
int all_count;		//모든토큰 담는 카운트
int ans_big_count;
int ans_all_count;	//답지 모든토큰 담는 카운트

int opr_count;		//연산자 담는 카운트

int ans_opr_count;	//답지 연산자 담는 카운트

int tok_count;

double blankcheck(int student_count, int answer_count, double score)
{
	int i,j,k;
	int ans_g_count = 0;
	int std_g_count = 0;

	count = 0;
	big_count = 1;
	all_count = 0;
	ans_big_count = 0;
	ans_all_count = 0;
	opr_count = 0;
	ans_opr_count = 0;
	tok_count = 0;

	char std_buf[BUFFER_SIZE] = {0};
	char ans_buf[BUFFER_SIZE] = {0};

	sprintf(path, "./%s/%s/%s.txt",std_dir, Students[student_count].stu_num, Questions[answer_count].q_name);
	if((std_blank_fd = open(path, O_RDONLY | O_APPEND)) < 0){
		//파일이없음 = 0점
		close(std_blank_fd);
		return 0.0;
	}
	
	if(lseek(std_blank_fd, 0, SEEK_END) == 0)	//빈 파일은 0점
		return 0.0;

	lseek(std_blank_fd, 0, SEEK_SET);

	read(std_blank_fd, std_buf, BUFFER_SIZE);

	sprintf(path, "./%s/%s/%s.txt", ans_dir, Questions[answer_count].q_name, Questions[answer_count].q_name);
	if((ans_blank_fd = open(path, O_RDONLY | O_APPEND)) < 0){
		fprintf(stderr, "Not exist %s file\n", path);
		exit(1);
	}

	lseek(std_blank_fd, 0, SEEK_SET);

	read(ans_blank_fd, ans_buf, BUFFER_SIZE);

	lseek(std_blank_fd, 0, SEEK_SET);
	lseek(ans_blank_fd, 0, SEEK_SET);	//오프셋 처음으로


	//버퍼를 토큰으로 분리(학생폴더만)
	while(read(std_blank_fd, &t, 1)){	//공백무시 및 토큰화
		if(t == ' ' || t == '\n'){
			if(tok_count > 0){
				strcpy(all_token[all_count++], token);
				for(i=0; i<tok_count; i++)
					token[i] = 0;
				tok_count =0;
			}
			continue;
		}

		if(t == '(' || t == ')')
			std_g_count++;
		
		if(IS_CHARACTER){//연산자가 아니라면 이어감
			strcat(token, &t);
			tok_count++;
		}
		else{//연산자면 이전것 토큰화, 다음것을 확인 후 끊고 이전까지 토큰화
			if(tok_count > 0){	//이전에 토큰이 만들어졌으면 토큰을넣음
				strcpy(all_token[all_count++], token);	//all토큰에 담음

				for(i=0; i<tok_count; i++)	//토큰화되어 이동했으니 초기화
					token[i] = 0;
				tok_count = 0;		//토큰배열의 초기화완료
			}

			strcat(token, &t);
			tok_count++;
			if(read(std_blank_fd, &t, 1)){	//다음것을 읽어들임
				if(t == ' ' || t == '\n'){	//공백이었다면
					lseek(std_blank_fd, -1, SEEK_CUR); //읽기 취소
					if(tok_count > 0){	//이전에 담겼던 것을 토큰화
						strcpy(all_token[all_count++], token);
						strcpy(opr_token[opr_count++], token);

						for(i=0; i<tok_count; i++)
							token[i] = 0;
						tok_count = 0;
					}
					continue;
				}

				strcat(token, &t);	//token에 길이2의 문자열이 저장되고 그것을 비교
				tok_count++;
				for(i=0; i<TWO_OPERATOR_LENGTH; i++){
					if(strcmp(token, two_operator_list[i]) == 0){	// || 같은 이중 연산자일경우
						strcpy(all_token[all_count++], token);	//토큰으로 추가하고 초기화, 종료
						strcpy(opr_token[opr_count++], token);	//연산자 토큰으로 추가

						for(j=0; j<tok_count; j++)
							token[j] = 0;
						tok_count = 0;
						break;
					}
				}
				if(tok_count == 0)	//앞에서 토큰처리가 완료됬음
					continue;

				//여기까지오면 이중 연산자가 아니었음을 의미
				lseek(std_blank_fd, -1, SEEK_CUR);	//커서롤백
				token[--tok_count] = 0;	//새로 읽어왔던것 제거
				strcpy(all_token[all_count++], token);	//1개짜리 연산자를 추가
				strcpy(opr_token[opr_count++], token);	//1개짜리 연산자를 연산토큰으로 추가

				for(i=0; i<tok_count; i++)	//토큰버퍼 초기화
					token[i] = 0;
				tok_count = 0;
			}
		}

	}
	
	if(tok_count > 0){	//끝났으면 남은토큰도 넣어줌
		strcpy(all_token[all_count++], token);
		for(i=0; i<tok_count; i++)
			token[i] = 0;
		tok_count = 0;
	}

	//정답파일의 내용을 : 전까지만 버퍼에 담아서 토큰비교 시작
	char* ans_ptr = strtok(ans_buf, ":");
	do{
		int big_count_array[MAX] = {0};
		int rem_bigcount;
		int changeAct = 0;
		char rem_opr[MAX] = {0};
		char big_token[32][50][BUFFER_SIZE] = {0};
		char stack_token[BUFFER_SIZE];
		int big_size = 1;
		int correct = 0;
		
		char ans_all_token[MAX][BUFFER_SIZE] = {0};
		int ans_length = strlen(ans_ptr);
		ans_all_count = 0;
		tok_count = 0;
		ans_g_count = 0;

		char ans_opr_token[MAX][10] = {0};      //답지의 연산자 담는 토큰
		ans_opr_count = 0;

		for(i=0; i<ans_length; i++){	//문자열 하나씩 비교 시작
			t = ans_ptr[i];
			if(t == '(' || t == ')')
				ans_g_count++;

			if(t == ' ' || t == '\n'){
				if(tok_count > 0){
					strcpy(ans_all_token[ans_all_count++], token);
					for(j=0; j<tok_count; j++)
						token[j] = 0;
					tok_count = 0;
				}
				continue;
			}

			if(IS_CHARACTER){	//문자면 스킵
				strcat(token, &t);
				tok_count++;
			}
			else{		//연산자
				if(tok_count > 0){	//이전토큰이 존재했다면 채움
					strcpy(ans_all_token[ans_all_count++], token);

					for(j=0; j<tok_count; j++)
						token[j] = 0;
					tok_count = 0;
				}

				strcat(token, &t);		//연산자 읽어들임
				tok_count++;

				if(i != ans_length-1){	//끝쪽이 아니라면
					t = ans_ptr[i+1];	//다음토큰을 확인한다

					if(t == ' ' || t == '\n'){
						if(tok_count > 0){
							strcpy(ans_all_token[ans_all_count++], token);
							for(j=0; j<tok_count; j++)
								token[j] = 0;
							tok_count = 0;
						}
						continue;
					}
					
					strcat(token, &t);
					tok_count++;

					for(j=0; j<TWO_OPERATOR_LENGTH; j++){//이중연산자
						if(strcmp(token, two_operator_list[j]) == 0){
							strcpy(ans_all_token[ans_all_count++], token);	
							strcpy(ans_opr_token[ans_opr_count++], token);

							for(k=0; k<tok_count; k++)
								token[k] = 0;
							tok_count = 0;
							i++;	//이중이었으니 스킵함
						}	
					}
					
					if(tok_count == 0)	//버퍼가 비었으면 아래는 할필요없다.
						continue;

					//여기서부턴 단일 연산자였음을 나타냄 

					token[--tok_count] = 0;	//새로 읽어왔던것 제거
					strcpy(ans_all_token[ans_all_count++], token);	//1개짜리 연산자를 추가
					strcpy(ans_opr_token[ans_opr_count++], token);	//1개짜리 연산자를 연산토큰으로 추가

					for(j=0; j<tok_count; j++)	//토큰버퍼 초기화
						token[j] = 0;
					tok_count = 0;

				}	//if중괄호
			}	//else 중괄호
		}	//for문 중괄호
		
		if(tok_count > 0){	//토큰남아있으면 처리
			strcpy(ans_all_token[ans_all_count++], token);
			for(j=0; j<tok_count; j++)
				token[j] = 0;
			tok_count = 0;
		}
		
		//여기서부터 처리를 시작함
		//현재 답지의 :이전까지를 전부토큰화, 학생이 낸 답을 전부 토큰화하였음

		correct = 0;
		for(i=0; i<all_count; i++){	//토큰 위치별로 1:1매칭이되는지 체크
			if(strcmp(all_token[i], ans_all_token[i]) == 0){
				correct++;
			}
		}

		if(correct == all_count && all_count == ans_all_count){//1:1매칭이 성공적으로 이루어지면 정답임
			close(std_blank_fd);
			close(ans_blank_fd);
			return score;
		}
		else{	//별도의 채점과정으로 이동
			for(i=0; i<all_count; i++){
				for(j=0; j<CHANGE_LENGTH; j++){
					if(strcmp(all_token[i],change_operator_list[j]) == 0){
						changeAct = 1;
						strcpy(rem_opr, all_token[i]);
						break;
					}
				}

				if(changeAct == 0){	//일반문자열이면 그냥 저장
					for(j=1; j<=big_size; j++){
						strcpy(big_token[j][++big_count_array[j]], all_token[i]);
					}
				}
				else{
					rem_bigcount = big_size;
					big_size*=2;	//복사본필요

					if(strcmp(">",rem_opr) == 0){ //각각 교환되었을때의 토큰을 추가함
						for(j=rem_bigcount+1; j<=big_size; j++){
							for(k=1; k<=big_count_array[j-rem_bigcount]; k++)
							{
								strcpy(big_token[j][k], big_token[j-rem_bigcount][k]);
								if(k == big_count_array[j-rem_bigcount]){
									strcpy(big_token[j][k+1],"<");
									big_count_array[j] = big_count_array[j-rem_bigcount] + 1;
								}
							}
						}
					}
					else if(strcmp(">=",rem_opr) == 0){
						for(j=rem_bigcount+1; j<=big_size; j++){
							for(k=1; k<=big_count_array[j-rem_bigcount]; k++)
							{
								strcpy(big_token[j][k], big_token[j-rem_bigcount][k]);
								if(k == big_count_array[j-rem_bigcount]){
									strcpy(big_token[j][k+1],"<=");
									big_count_array[j] = big_count_array[j-rem_bigcount] + 1;
								}
							}
						}
					}
					else if(strcmp("<",rem_opr) == 0){
						for(j=rem_bigcount+1; j<=big_size; j++){
							for(k=1; k<=big_count_array[j-rem_bigcount]; k++)
							{
								strcpy(big_token[j][k], big_token[j-rem_bigcount][k]);
								if(k == big_count_array[j-rem_bigcount]){
									strcpy(big_token[j][k+1],">");
									big_count_array[j] = big_count_array[j-rem_bigcount] + 1;
								}
							}
						}
					}
					else if(strcmp("<=",rem_opr) == 0){
						for(j=rem_bigcount+1; j<=big_size; j++){
							for(k=1; k<=big_count_array[j-rem_bigcount]; k++)
							{
								strcpy(big_token[j][k], big_token[j-rem_bigcount][k]);
								if(k == big_count_array[j-rem_bigcount]){
									strcpy(big_token[j][k+1],">=");
									big_count_array[j] = big_count_array[j-rem_bigcount] + 1;
								}
							}
						}
					}

					for(j=1; j<=rem_bigcount; j++)
						strcpy(big_token[j][++big_count_array[j]], all_token[i]);
					changeAct = 0;

					strcpy(rem_opr,"");

				}
			}
		}
		//매칭시작

		int OK = 0;
		char copy_took[MAX][BUFFER_SIZE];
		char copy_big_took[32][50][BUFFER_SIZE];
		for(i=0; i<ans_all_count; i++)
			strcpy(copy_took[i], ans_all_token[i]);


		for(i=1; i<=big_size; i++){
			for(j=1; j<=big_count_array[i]; j++){
				strcpy(copy_big_took[i][j], big_token[i][j]);
			}
		}
		//교환으로 발생한 모든 경우를 비교하며 모든 토큰을 가졌는지 체크
		for(i=1; i<=big_size; i++){
			for(j=0; j<ans_all_count; j++){
				for(k=1; k<=big_count_array[i]; k++){
					if(strcmp(copy_big_took[i][k], copy_took[j]) == 0){
						strcpy(copy_took[j], "DUMMY__@");
						strcpy(copy_big_took[i][k], "DUMMY__@");
						OK++;
						break;
					}
				}

				for(k=0; k<ans_all_count; k++)
					strcpy(copy_took[k], ans_all_token[k]);
			}

			if((OK == ans_all_count) && (ans_all_count - ans_g_count == all_count - std_g_count)){	//해당 리스트에 모두 들어있고 토큰 개수까지 동일하면 정답
				close(std_blank_fd);
				close(ans_blank_fd);
				return score;
			}
			else	//하나라도 없었다면 오답이고 다음리스트를 확인
				OK = 0;
		}

	}while((ans_ptr = strtok(NULL, ":")) != NULL);	//NULL이면 멈춤
	
	close(std_blank_fd);
	close(ans_blank_fd);
	return 0.0;	//정답해당이 안되므로 0점임
}
