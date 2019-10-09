// Programing by Hyoga.Fukuno
// Date: 2019/ 10/ 7
#ifndef __SCENEMANAGER_H__
#define __SCENEMANAGER_H__
// --------------- IncludeFile ---------------
#include "Global.h"
#include "BaseScene.h"
#include "DirectXinclude.h"
#include "InputDevice.h"
// ---------------- クラス宣言 ---------------
typedef struct _VERTEX_
{
	VECTOR4 vPosition;
	VECTOR4 vColor;
}VERTEX;
class CSceneManager : public CSceneChanger, public CSceneModule
{
private:
	HWND						m_hWnd;			// ウィンドウハンドル
	ID3D12Device*				m_pd3dDevice;	// D3D12デバイス
	ID3D12GraphicsCommandList*	m_pCommandList;
	ID3D12RootSignature*		m_pRootSignature;
	ID3D12PipelineState*		m_pPipelineState;
	CBaseScene*					m_pScene;		// シーン
	eScenes						m_NextScene;	// 次切り替えるシーン

	ID3D12Resource*				m_pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW	m_VertexBufferView;
public:
	CSceneManager(HWND, ID3D12Device*,
		ID3D12GraphicsCommandList*);		// コンストラクタ
	~CSceneManager();						// デストラクタ
	HRESULT Initialize() override;
	void Render() override;
	void ChangeScene(eScenes) override;
	ID3D12PipelineState* GetPipelineState() { return m_pPipelineState; }
};
#endif /* __SCENEMANAGER_H__ */