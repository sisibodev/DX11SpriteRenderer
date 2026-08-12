# DX11 2D 스프라이트 렌더러 — 아키텍처 문서

> 작성 중인 문서입니다. Phase가 끝날 때마다 해당 장을 채웁니다.
> `[확인 필요]` 표시는 본인 판단·경험을 직접 채워야 하는 부분입니다.

---

## 1. 개요

### 1.1 목적

DirectX 11을 직접 사용해 2D 스프라이트 렌더러를 구현한 프로젝트다.
핵심 목표는 **스프라이트 배칭을 통한 드로우콜 최소화**이며, 이를 통해 렌더링 파이프라인과
성능 최적화의 원리를 코드 수준에서 다룬다.

- **개발 기간**: 2026-08-10 ~ 2026-08-23 (14일)
- **개발 인원**: 1인
- **개발 동기**: 13년간 상용 엔진(게임브리오, Unity) 위에서 클라이언트를 개발해왔으나
  그 아래의 렌더링 파이프라인을 직접 다뤄본 경험은 없었다. 그 공백을 메우기 위해
  엔진에 의존하지 않고 렌더러를 처음부터 구현했다.

### 1.2 기술 스택

| 항목 | 내용 |
|---|---|
| 언어 | C++17 |
| 그래픽 API | Direct3D 11 (Feature Level 11.0) |
| 개발 환경 | Visual Studio 2022, Windows 11 (x64) |
| 외부 의존성 | `stb_image` (단일 헤더, 퍼블릭 도메인) |
| 에셋 | Kenney.nl (CC0) |

외부 라이브러리는 **의도적으로 최소화**했다. 특히 `DirectXTK`는 `SpriteBatch`를 이미 제공하므로,
배칭을 직접 구현한다는 프로젝트 목적과 맞지 않아 사용하지 않았다.

### 1.3 실행 방법

```
1. Visual Studio 2022로 DX11SpriteRenderer.sln 열기
2. 구성: Debug 또는 Release / 플랫폼: x64
3. F5 실행
```

셰이더와 에셋은 빌드 후 이벤트로 출력 폴더에 복사되며, 실행 파일 단독으로도 동작한다.

**조작**: `[확인 필요 — 키 조작이 생기면 여기에 추가. 현재는 ESC 종료뿐]`

---

## 2. 아키텍처

### 2.1 전체 구조

```
src/
├─ main.cpp              진입점 (13줄)
├─ core/
│  ├─ App               생명주기 관리 (Initialize / Run / Update / Render)
│  └─ Window            Win32 창 생성, 메시지 루프
├─ render/
│  ├─ GraphicsDevice    D3D11 디바이스·스왑체인·렌더 상태
│  ├─ Shader            HLSL 런타임 컴파일, 파이프라인 바인딩
│  ├─ Texture           이미지 로드 → GPU 텍스처
│  └─ Vertex.h          정점 구조체 + 입력 레이아웃 정의
├─ shaders/             HLSL 소스 (런타임 컴파일)
└─ third_party/         stb_image
```

### 2.2 계층 규칙

의존 방향을 **한쪽으로만** 흐르게 제한했다.

```
main → App → { Window, GraphicsDevice, Shader, Texture }
```

- `render/` 계층은 `core/`를 알지 못한다 (하위가 상위를 참조하지 않음)
- `App`은 조립과 흐름 제어만 담당하고, 실제 작업은 각 클래스에 위임한다
- COM 객체의 수명은 전부 `ComPtr`(RAII)로 관리한다

`App`의 멤버 선언 순서는 소멸 순서를 결정한다. 멤버는 선언의 **역순**으로 소멸하므로,
`Window`를 가장 먼저 선언해 D3D 리소스들이 모두 정리된 뒤에 창이 닫히도록 했다.

### 2.3 렌더링 파이프라인 흐름

```
[정점 버퍼] ─→ IA (Input Assembler)
                 │  InputLayout이 바이트 배열을 정점 구조로 해석
                 ↓
              VS (Vertex Shader)     정점 위치 계산 → SV_POSITION
                 ↓
              RS (Rasterizer)        삼각형 → 픽셀, 정점 값 보간
                 ↓
              PS (Pixel Shader)      텍스처 샘플링 → 최종 색
                 ↓
              OM (Output Merger)     블렌딩 후 렌더타겟에 기록
```

정점에 넣은 색이 삼각형 내부에서 그라데이션으로 나타나는 것은 **RS 단계의 보간** 때문이다.
같은 원리로 UV도 보간되어, 사각형 전체에 텍스처가 늘어나 붙는다.

---

## 3. 핵심 구현

### 3.1 GraphicsDevice — D3D11 초기화

#### 세 객체의 역할 분리

| 객체 | 역할 |
|---|---|
| `ID3D11Device` | 리소스 **생성** (버퍼, 텍스처, 셰이더, 상태 객체) |
| `ID3D11DeviceContext` | 렌더 **명령 발행** (바인딩, 드로우, 클리어) |
| `IDXGISwapChain` | 화면 버퍼 관리 및 `Present` |

이렇게 나뉜 이유는 **멀티스레딩** 때문이다. `Device`는 스레드 안전해 여러 스레드에서 리소스를
생성할 수 있지만, `DeviceContext`는 그렇지 않아 명령 발행은 한 스레드에서 이뤄져야 한다.

#### 초기화 순서

```
1. DXGI_SWAP_CHAIN_DESC 구성
2. D3D11CreateDeviceAndSwapChain  → Device + Context + SwapChain
3. SwapChain::GetBuffer           → 백버퍼(ID3D11Texture2D) 획득
4. CreateRenderTargetView         → 백버퍼를 '그릴 대상'으로 지정
5. RSSetViewports                 → NDC를 픽셀 좌표로 매핑할 영역
6. CreateSamplerState             → 텍스처 읽기 방식
7. CreateBlendState               → 알파 합성 방식
```

3~4단계가 나뉘어 있는 이유는, 백버퍼 자체는 단순한 텍스처이고 **"이 텍스처를 렌더타겟으로
쓰겠다"는 용도 선언이 View**이기 때문이다. 같은 텍스처라도 용도(렌더타겟 / 셰이더 입력)에 따라
다른 View를 만들어 붙인다.

#### 스왑 효과 — FLIP 모델에서 DISCARD로

처음에는 `DXGI_SWAP_EFFECT_FLIP_DISCARD`(플립 모델)로 스왑체인을 구성했으나
`DXGI_ERROR_INVALID_CALL`로 생성에 실패했다.

원인을 추적한 결과, `D3D11CreateDeviceAndSwapChain`이 내부적으로 **구형 DXGI 1.0 경로**를
사용하기 때문이었다. 플립 모델을 쓰려면 디바이스를 먼저 생성한 뒤
`IDXGIFactory2::CreateSwapChainForHwnd`로 스왑체인을 따로 만들어야 한다.

당시 필요한 것은 화면 출력이 되는 상태였으므로 `DISCARD`(비트블릿 모델)로 전환해 진행했고,
플립 모델 적용은 개선 항목으로 남겼다(5장 참고). 두 모델의 차이는 전체화면 전환의 매끄러움과
화면 합성 단계의 복사 횟수이며, 이 프로젝트 규모에서는 체감되는 차이가 없었다.

#### 렌더 상태의 소유권

`SamplerState`와 `BlendState`를 `GraphicsDevice`가 소유하고 `BeginFrame`에서 바인딩한다.
2D 스프라이트 렌더러에서는 이 두 상태가 **전체에 걸쳐 동일**하므로, 매 드로우마다 다루기보다
디바이스 수준의 기본 상태로 두는 편이 단순하다.

`[확인 필요 — 나중에 가산 블렌딩(이펙트용)을 지원하게 되면 이 판단이 바뀐다.
그때 이 문단을 갱신할 것]`

### 3.2 Shader — 런타임 컴파일

셰이더를 빌드 시점이 아닌 **런타임에 컴파일**한다(`D3DCompileFromFile`).
셰이더만 수정하고 다시 실행하면 되므로 반복 속도가 빠르고, 컴파일 오류를 실행 중에
확인할 수 있다.

가장 중요한 부분은 **에러 블롭 출력**이다.

```cpp
if (FAILED(hr))
{
    if (errorBlob)
        printf("[셰이더 컴파일 실패]\n%s\n",
               static_cast<const char*>(errorBlob->GetBufferPointer()));
}
```

이 출력이 없으면 "컴파일에 실패했다"는 사실만 알 뿐 원인을 알 수 없다.
`ID3DBlob`은 크기 정보를 가진 이진 데이터 덩어리로, 여기서는 에러 메시지를,
성공 시에는 컴파일된 셰이더 바이트코드를 담는다.

#### InputLayout이 셰이더 바이트코드를 요구하는 이유

```cpp
device->CreateInputLayout(layout, layoutCount,
                          vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                          &m_inputLayout);
```

D3D가 **선언한 정점 구조와 셰이더의 실제 입력 시그니처가 일치하는지 생성 시점에 검증**하기
위해서다. 시맨틱 이름이나 개수가 맞지 않으면 여기서 실패하므로, 런타임에 화면이 깨지는 대신
초기화 단계에서 문제를 잡을 수 있다.

### 3.3 Texture — 이미지 로드

```
stb_image로 디코드 (RGBA 4채널 강제)
  → CreateTexture2D (D3D11_USAGE_IMMUTABLE)
  → stbi_image_free (CPU 메모리 해제)
  → CreateShaderResourceView
```

- **4채널 강제**: D3D에는 24비트 RGB 포맷이 없어 항상 RGBA로 변환한다
- **`SysMemPitch`**: 이미지 한 줄의 바이트 수(`너비 × 4`). 이 값이 틀리면 이미지가
  대각선으로 밀려 나타난다
- **`USAGE_IMMUTABLE`**: 업로드 후 변경하지 않으므로 GPU가 가장 효율적으로 배치할 수 있다
- `ID3D11Texture2D` 핸들은 보관하지 않는다. `ShaderResourceView`가 참조를 유지하고,
  셰이더에서 쓰려면 어차피 View가 필요하기 때문이다

#### 텍스처와 샘플러를 분리하는 이유

텍스처는 *무엇을 읽는가*, 샘플러는 *어떻게 읽는가*를 담당한다.
같은 텍스처를 선명하게도 부드럽게도 읽을 수 있어야 하고, 반대로 여러 텍스처가 같은 읽기
방식을 공유하는 경우가 대부분이다. 그래서 샘플러는 몇 개만 만들어 재사용한다.

이 프로젝트는 `D3D11_FILTER_MIN_MAG_MIP_POINT`를 선택했다.
`[확인 필요 — POINT를 고른 이유. 예: 픽셀 아트의 선명함을 유지하기 위해 등]`

---

## 4. 최적화

`[Day 10 이후 작성 — 배칭 전/후 드로우콜·FPS 측정 결과]`

---

## 5. 회고

### 프로젝트 진행 방식

2주라는 제약 안에서 처음 다루는 스택을 익혀야 했기에, 학습 도구로 AI를 적극 활용했다.
다만 코드 생성에 의존하기보다 **단계별 일정을 먼저 설계하고, 각 단계에서 필요한 개념을
이해한 뒤 직접 구현하는 방식**을 택했다. 발생한 문제는 증상에서 멈추지 않고 원인을 규명해
아래에 정리했다.

결과적으로 이 프로젝트에서 얻은 것은 렌더러 코드 자체보다, **모르는 영역을 짧은 기간에
실무 수준까지 끌어올리는 방법**에 가깝다.

### 겪은 문제와 해결

`[확인 필요 — 아래는 실제로 겪은 것들의 메모. 본인 언어로 다듬을 것]`

**① C++와 HLSL의 인코딩 요구가 정반대였다**
MSVC는 BOM이 없으면 파일을 시스템 코드페이지(CP949)로 해석해 한글 주석이 깨진다.
반대로 셰이더 컴파일러(FXC)는 BOM이 있으면 `error X3000: Illegal character`로 실패한다.
`.editorconfig`에서 확장자별로 인코딩을 분리 지정해 해결했다.

**② 0으로 초기화한 구조체의 함정**
`DXGI_SWAP_CHAIN_DESC`에서 `OutputWindow`와 `Windowed`를 채우지 않아
`DXGI_ERROR_INVALID_CALL`이 발생했다. `= {}`로 0 초기화하면 `Windowed = FALSE`가 되는데,
이는 "전체화면"이라는 **유효한 값**이라 컴파일러가 잡아줄 수 없었다.
D3D 설정 구조체는 필드를 하나씩 짚어가며 채웠는지 확인하는 습관이 필요하다고 느꼈다.

**③ 만들었지만 바인딩하지 않은 리소스**
`SamplerState`를 생성해두고 `PSSetSamplers` 호출을 누락했다. 슬롯이 비면 D3D가 기본
샘플러를 사용하므로 **에러도 없고 화면도 정상이지만, 의도한 설정이 무시되는** 상태였다.
동작한다고 해서 의도대로 동작하는 것은 아니라는 걸 확인한 사례다.

### 개선하고 싶은 부분

`[확인 필요 — 본인 판단으로 채울 것. 후보:]`
- `GraphicsDevice.h`가 D3D11 의존성을 외부로 노출한다. 규모가 커지면 PIMPL로 감출 수 있으나
  현 규모에서는 복잡도 대비 이득이 적다고 판단해 보류했다.
- 플립 모델 스왑체인(`CreateSwapChainForHwnd`) 적용
- 가산 블렌딩 등 블렌드 모드 전환 지원

---

## 부록: 참고

- 에셋: [Kenney.nl](https://kenney.nl) (CC0)
- 이미지 디코딩: [stb_image](https://github.com/nothings/stb) (Public Domain)
