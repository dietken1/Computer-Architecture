# 컴퓨터 구조 PA2 보고서

이 보고서는 **MIPS 아키텍처**를 구현한 프로그램에 대한 설명과 개발 과정입니다. 해당 프로젝트는 MIPS 명령어의 다양한 형식을 지원하며, 프로그램의 각 기능을 어떻게 구현했는지에 대한 설명을 포함하고 있습니다. 주로 **R-format**, **I-format**, **J-format** 명령어 처리와 관련된 내용을 다루고 있습니다.

---

## 목차

1. [프로젝트 개요](#프로젝트-개요)
2. [주요 함수 구현](#주요-함수-구현)
   - [process_instruction 함수](#process_instruction-함수)
   - [R-format / I, J–format 명령어 구분](#r-format--i-jformat-명령어-구분)
   - [R-format 명령어 처리](#r-format-명령어-처리)
   - [I-format 명령어 처리](#i-format-명령어-처리)
   - [J-format 명령어 처리](#j-format-명령어-처리)
3. [핵심 함수 설명](#핵심-함수-설명)
   - [load_program 함수](#load_program-함수)
   - [run_program 함수](#run_program-함수)
4. [문제 해결 과정](#문제-해결-과정)
   - [엔디안 방식 문제 해결](#엔디안-방식-문제-해결)
   - [jal 명령어 문제 해결](#jal-명령어-문제-해결)
5. [프로그램 분석](#프로그램-분석)
   - [program-hidden 분석](#program-hidden-분석)
6. [배운 점](#배운-점)

---

## 프로젝트 개요

이 프로젝트는 MIPS 아키텍처를 기반으로 한 간단한 CPU 에뮬레이터입니다. MIPS 어셈블리 언어의 **R-format**, **I-format**, **J-format** 명령어를 구현하고, 프로그램을 메모리에 로드하여 실행하는 기능을 제공합니다. 이 과정에서 MIPS 명령어를 처리하고, 엔디안 문제를 해결하며, 프로그램을 로드하고 실행하는 등의 중요한 개념을 학습할 수 있었습니다.

---

## 주요 함수 구현

### process_instruction 함수

이 함수는 명령어를 실행하는 주요 함수로, 각 명령어를 해석하여 적절한 연산을 수행합니다. **opcode** 값을 기준으로 명령어를 분류하고, R-format, I-format, J-format으로 나누어 처리합니다. 각 형식에 맞는 비트 마스킹과 연산을 통해 명령어를 실행합니다.

### R-format / I, J–format 명령어 구분

MIPS 어셈블리의 각 명령어는 **opcode** 값을 가집니다. 이 값을 바탕으로 명령어를 크게 두 가지로 분류할 수 있습니다: 

1. **R-format**: opcode 값이 0인 명령어.
2. **I-format 및 J-format**: 그 외의 명령어들.

R-format 명령어는 추가적으로 **funct** 값을 사용하여 세부적으로 구분합니다.

### R-format 명령어 처리

R-format 명령어들은 비트 마스킹을 통해 각 필드를 추출하고, 이를 레지스터 배열에 인덱스를 사용하여 연산을 처리합니다. 특히 **mult**, **mfhi**, **mflo**와 같은 명령어를 처리하는 데 있어 **64비트** 연산을 고려해야 했습니다. 이 명령어들을 위해 **hi**와 **lo** 레지스터를 사용하고, 비트 확장을 통해 곱셈 결과를 처리합니다.

### I-format 명령어 처리

I-format 명령어는 상숫값을 포함한 명령어입니다. **lw**와 **sw**와 같은 메모리 연산 명령어를 처리할 때, 메모리 주소를 적절히 계산하고, 비트 마스킹을 통해 데이터를 정확하게 읽고 씁니다. 부호 확장을 고려하여 음수 및 양수 값을 처리하는 로직이 포함되어 있습니다.

### J-format 명령어 처리

J-format 명령어는 **j**와 **jal** 명령어를 포함합니다. **j** 명령어는 점프할 주소를 계산할 때, 현재 **PC**(Program Counter)의 상위 4비트를 유지하고 하위 26비트를 사용합니다. **jal** 명령어의 경우, 현재 **PC** 값에 4를 더한 후 이를 **ra**(return address) 레지스터에 저장해야 하므로 이를 처리하는 데 신경을 썼습니다.

---

## 핵심 함수 설명

### load_program 함수

이 함수는 프로그램을 메모리에 로드하는 함수로, 사용자가 입력한 파일을 읽고 각 명령어를 메모리에 로드합니다. 여기서 중요한 점은 **주석 처리된 명령어를 제외하고 실제 명령어만을 추출**하는 작업이었으며, 이를 위해 `strtoul` 함수를 사용하여 16진수 값을 변환하고 메모리에 저장하였습니다.

### run_program 함수

**process_instruction** 함수를 호출하여 명령어를 실행하는 함수입니다. 프로그램이 정상적으로 실행되도록 **Fetch-Decode-Execute** 순서에 맞게 명령어를 처리하고, 프로그램이 종료될 때까지 계속해서 명령어를 실행합니다. 이 과정에서 **엔디안 문제**를 해결하여 메모리에서 값을 올바르게 불러오도록 설계하였습니다.

---

## 문제 해결 과정

### 엔디안 방식 문제 해결

MIPS 아키텍처는 **빅 엔디안** 방식을 사용합니다. 하지만, 저의 개발 환경은 **리틀 엔디안** 방식이기 때문에, 메모리에서 값을 불러올 때 값이 뒤집히는 문제가 발생했습니다. 이를 해결하기 위해, **파일에서 읽어온 값을 비트 순서를 반대로 바꿔주는 방식**으로 문제를 해결하였습니다.

### jal 명령어 문제 해결

**jal** 명령어가 예상한 대로 작동하지 않는 문제를 겪었습니다. **PC에 4를 더한 값을 ra 레지스터에 저장해야** 하는데, 이 부분에서 착오가 있었음을 깨닫고, 코드 로직을 수정하여 문제를 해결하였습니다.

---

## 프로그램 분석

### program-hidden 분석

프로그램을 분석하는 과정에서, **branch 명령어**를 기준으로 함수들을 나누어 각 명령어의 흐름을 추적했습니다. 이 과정을 통해, 프로그램이 메모리에서 값을 읽어오는 방식과 **비트 마스킹**을 통해 데이터를 처리하는 방법을 명확히 이해할 수 있었습니다.

---

## 배운 점

이번 과제를 통해 MIPS 아키텍처의 동작 방식을 깊이 이해할 수 있었습니다. 특히, **비트 연산**과 **메모리 관리**의 중요성을 실감했으며, **엔디안** 방식을 비롯한 다양한 시스템 간 차이점을 깨닫게 되었습니다. 또한, **컴파일러**가 하는 역할과 **컴퓨터 구조**에 대한 이해도를 높이는 중요한 경험이었습니다.
