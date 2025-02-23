#include <ctype.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);   // 프로그램이 종료되는 시점에 트리거 되는 함수 등록(golang의 defer 비슷한 느낌)

  struct termios raw = orig_termios;

  // 입력 플래그 설정
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // BRKINT: Break condition 시 신호 중지
  // ICRNL: Carriage return을 newline으로 변환하지 않음
  // INPCK: Parity check 비활성화
  // ISTRIP: 8번째 비트 strip 비활성화
  // IXON: 소프트웨어 흐름 제어 비활성화

  // 출력 플래그 설정
  raw.c_oflag &= ~(OPOST);
  // OPOST: 포스트 프로세싱 출력 비활성화

  // 제어 플래그 설정
  raw.c_cflag |= (CS8);
  // CS8: 8비트 바이트로 캐릭터 크기 설정

  // 로컬 플래그 설정
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  // ECHO: 에코 비활성화
  // ICANON: 캐논 모드 비활성화, 입력을 즉시 처리
  // IEXTEN: 확장 입력 처리 비활성화
  // ISIG: 시그널 발생 비활성화

  raw.c_cc[VMIN] = 0;     // VMIN (Minimum number of characters to read). 0이면 읽을 게 없어도 바로 리턴
  raw.c_cc[VTIME] = 1;    // VTIME (Time to wait for data)

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int main()
{
  enableRawMode();

  while (1) {
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') break;
  }


  return 0;
}
