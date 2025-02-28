# 컴퓨터 구조 PA0 보고서

## 1. 개요

이 보고서는 소프트웨어학과 정원준 학생이 구현한 **하드웨어적 계산기**의 동작 원리와 코드 구현 방법에 대한 설명입니다. 본 프로그램은 문자열 형태로 표현된 계산식을 읽어들여 정수 토큰을 처리하고, 덧셈과 뺄셈 연산을 수행하여 최종 결과를 계산하는 기능을 구현하고 있습니다.

## 2. 프로그램의 주요 기능

이 프로그램은 주어진 계산식을 처리하는 기본적인 계산기 역할을 합니다. 특히, 다음과 같은 기능들을 중심으로 설계되었습니다.

- 문자열로 주어진 정수 토큰을 실제 정수로 변환
- `+`와 `-` 연산자의 처리
- 정수 토큰과 연산자를 순차적으로 처리하여 계산 수행

## 3. 주요 함수 설명

### 3.1 `intChange` 함수

`intChange` 함수는 문자열 형태의 숫자 토큰을 실제 정수로 변환하는 역할을 합니다. 이 함수는 아스키 코드 값을 이용해 각 문자를 숫자로 변환하며, 다자릿수 숫자에 대해서도 올바른 정수값을 계산할 수 있도록 설계되었습니다.

#### 작동 원리

- 문자 `'0'`의 아스키 값은 48, `'1'`의 아스키 값은 49, ... `'9'`의 아스키 값은 57입니다.
- 각 문자를 숫자로 변환하기 위해, 해당 문자에서 `'0'`의 아스키 값(48)을 빼면 숫자 값이 계산됩니다.
- 여러 자리 숫자에 대해서는 앞자리부터 차례대로 10배씩 곱한 뒤 더하는 방식으로 처리하여, 다자리 숫자도 정확하게 계산할 수 있습니다.

### 3.2 `do_compute` 함수

`do_compute` 함수는 주어진 토큰을 읽어들이고, 그에 맞춰 연산을 수행하는 함수입니다. 이 함수는 다음과 같은 주요 기능을 수행합니다.

- `+` 또는 `-` 연산자가 나올 때마다 연산 모드를 전환
- 정수 토큰을 읽으면 `intChange` 함수를 호출하여 정수로 변환
- 연산 모드에 따라 현재 계산된 값에 정수 값을 더하거나 빼기
- 최종적으로 계산된 결과를 반환

#### 작동 원리

- `+` 또는 `-` 연산자가 등장할 때마다 연산 모드를 변경하여 연산자가 바뀔 때마다 올바르게 더하거나 빼는 방식으로 동작합니다.
- 계산된 결과는 최종적으로 반환되어 main 함수에서 출력됩니다.

### 3.3 `main` 함수

`main` 함수는 프로그램의 진입점으로, 전체 프로그램의 흐름을 제어합니다. 이 함수는 `do_compute` 함수로부터 반환된 결과를 출력하며, 사용자가 입력한 계산식에 대해 연산을 수행합니다.

## 4. 코드 흐름 설명

1. **입력**: 문자열 형태의 계산식이 입력됩니다.
2. **토큰화**: 계산식에서 정수 토큰과 연산자 토큰을 구분하여 읽어들입니다.
3. **연산 수행**: 정수 토큰을 읽으면 `intChange` 함수를 호출하여 해당 숫자를 계산하고, 연산자(`+`, `-`)가 등장하면 `do_compute` 함수가 계산을 수행합니다.
4. **결과 출력**: 계산된 최종 결과를 `main` 함수에서 출력합니다.

## 5. 알고리즘의 설명

### 5.1 문자열을 정수로 변환하는 방식

문자열 형태로 주어진 숫자는 각 자릿수를 아스키 코드로 변환하여 실제 정수 값으로 바꿔주어야 합니다. 예를 들어, `'123'`이라는 문자열이 주어지면 각 자리를 계산하여 `123`이라는 정수로 바꿔주게 됩니다.
```c
intChange("123")  // 반환값: 123
```
### 5.2 연산자에 따른 계산 처리
do_compute 함수는 연산자가 +일 때는 더하고, -일 때는 빼는 방식으로 연산을 처리합니다. 이를 통해 계산을 순차적으로 수행할 수 있습니다.
```c
do_compute("5 + 10 - 3")  // 반환값: 12
```
