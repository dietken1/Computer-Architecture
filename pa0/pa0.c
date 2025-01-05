#include <stdio.h>
#include <string.h>
#include <stdlib.h>

 /* To avoid security error on Visual Studio */
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#define MAX_NR_TOKENS 32	/* Maximum number of tokens in a command */
#define MAX_TOKEN_LEN 64	/* Maximum length of single token */
#define MAX_COMMAND	256		/* Maximum length of command string */

static int parse_command(char* command, char* tokens[])
{
    int nr = 0;
    const char* separator = " 	";

    for (char* w = strtok(command, separator); w; w = strtok(NULL, separator)) {
        tokens[nr++] = strdup(w);
    }

    tokens[nr] = "\0";

    return nr;
}

int intChange(char* ary) {  //정수토큰(문자열)을 정수로 바꾸어주는 함수
    int n = 0;  //ary의 인덱스를 나타내는 변수
    int result = 0;     //총 결과에 더해줄 수(정수 토큰을 정수로 바꾼 값)

    while (ary[n] >= '0' && ary[n] <= '9') {    //ary[n]이 아스키코드로 바꾸었을때 정수인지
            result = result * 10 + (ary[n] - '0');      //아스키코드 이용
            n++;
    }
    return result;  //변환한 정수 반환
}

static int do_compute(int nr_tokens, char* tokens[])
{
    char mode = '+';    //덧셈과 뺄셈을 결정하는 변수
    int num = 0;    //total에 계속해서 더해줄 값을 나타내는 변수
    int total = 0;  //결과값을 나타내는 변수
    for (int i = 0; i < nr_tokens; i++) {
        if (*tokens[i] == '+') { //더하기모드로 변경
            int num = intChange(tokens[i]);
            mode = '+';
        }
        else if (*tokens[i] == '-') {   //빼기모드로 변경
            mode = '-';
        }
        else {  //정수 토큰을 읽었을때 연산을 시행
            int num = intChange(tokens[i]);
            if (mode == '+') {
                total += num;
            }
            else {
                total -= num;
            }
        }
    }
    return total;   //결과값 반환
}

int main(int argc, char* const argv[])
{
    char line[MAX_COMMAND] = { '\0' };
    FILE* input = stdin;

    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            fprintf(stderr, "Unable to open file %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    while (fgets(line, sizeof(line), input)) {
        char* tokens[MAX_NR_TOKENS] = { NULL };
        int nr_tokens;

        nr_tokens = parse_command(line, tokens);

        fprintf(stderr, "%d\n", do_compute(nr_tokens, tokens));

        /* No worries about the memory leak by strdup() ;-) */
    }

    if (input != stdin) fclose(input);

    return EXIT_SUCCESS;
}
