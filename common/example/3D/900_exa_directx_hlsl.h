#if 0

//---------------------------------------------------------------------------------------------------------------
//
//
// DirectX HLSL シェーダ入門 三角形描画 ( ライティング無し テクスチャ有り )
//
//
//---------------------------------------------------------------------------------------------------------------

#include <time.h>
#include <string>
#include <numbers>
#include <functional>
#include <dxe.h>
#include "../ResourceConstantHedder.h"
#include "gm_main.h"

// ComPtr は Microsoft 製のスマートポインタ 
// DirectX が生成したデータはプログラマが Releaseメソッドをコールして解放処理を行うか
// ComPtr で自動解放をするかどちらかです
// ComPtr を使用したほうが解放が楽なので使いたいが
// DxLib の LONG_PTR 定義が邪魔なので一時的に無効にして ComPtr を使えるようにする
#undef LONG_PTR
#include <wrl.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <VertexTypes.h>
#include <CommonStates.h>
#include <WICTextureLoader.h>
template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

using namespace DirectX;

Shared<dxe::Camera> camera = nullptr;

Shared<dxe::Texture> texture = nullptr;

// 頂点バッファ
ComPtr<ID3D11Buffer>				vertex_buffer = nullptr;
// インデックスバッファ
ComPtr<ID3D11Buffer>				index_buffer = nullptr;
// 頂点シェーダー
ComPtr<ID3D11VertexShader>          vertex_shader = nullptr;
// ピクセルシェーダー
ComPtr<ID3D11PixelShader>           pixel_shader = nullptr;
// 頂点入力レイアウト
ComPtr<ID3D11InputLayout>           input_layout = nullptr;
// 定数バッファ
ComPtr<ID3D11Buffer>                constant_buffer = nullptr;

// テクスチャ
ComPtr<ID3D11Resource>				tex_res = nullptr;
ComPtr<ID3D11ShaderResourceView>	tex_srv = nullptr;


// シェーダ定数定義
typedef struct CBUFFER
{
	DirectX::XMMATRIX	mat_view_;  // ビュー 行列
	DirectX::XMMATRIX	mat_proj_;  // 射影 行列
	DirectX::XMMATRIX	mat_world_;  // ワールド 行列
} TEST_CBUFFER;

// 頂点データ定義
typedef struct VERTEX
{
	DirectX::XMFLOAT3 position_;
	DirectX::XMFLOAT3 normal_;
	DirectX::XMFLOAT2 tex_coord_;
} TEST_VERTEX;

// テスト用メッシュの頂点
const int MESH_VERTEX_NUM = 3;
TEST_VERTEX mesh_vertexs[MESH_VERTEX_NUM];

// 頂点を結ぶ順番を定義するインデックス配列
// 三角形の枚数 * 3 になり配列内の各値は頂点座標配列の配列番号になる事に注意してください
// インデックスデータを３つ区切りで三角形のポリゴンを構成します
const int INDEX_NUM = 3;
UINT mesh_vtx_indexs[INDEX_NUM] = { 0, 2, 1 };


// モデルデータとしてのトランスフォーム情報
tnl::Vector3 position;
tnl::Vector3 scale = { 1, 1, 1 };
tnl::Quaternion rotation;


//------------------------------------------------------------------------------------------------------------
// ゲーム起動時に１度だけ実行されます
void gameStart() {

	srand(time(0));
	SetBackgroundColor(32, 32, 32);

	// カメラ生成
	camera = std::make_shared<dxe::Camera>(DXE_WINDOW_WIDTH_F, DXE_WINDOW_HEIGHT_F);

	// 頂点座標設定
	mesh_vertexs[0].position_ = { 0, 100, 0 };
	mesh_vertexs[1].position_ = { 50, 0, 0 };
	mesh_vertexs[2].position_ = { -50, 0, 0 };

	// 法線設定
	mesh_vertexs[0].normal_ = { 0, 0, -1 };
	mesh_vertexs[1].normal_ = { 0, 0, -1 };
	mesh_vertexs[2].normal_ = { 0, 0, -1 };

	// UV 座標設定
	mesh_vertexs[0].tex_coord_ = { 0.5f, 0 };
	mesh_vertexs[1].tex_coord_ = { 1, 1 };
	mesh_vertexs[2].tex_coord_ = { 0, 1 };

	// DirectX 関数が共通で返す戻り値型
	// 関数のエラー検知で使用
	HRESULT hr = E_FAIL;

	// シェーダのコンパイル関数が返してくるエラーメッセージ受け取り用
	ID3DBlob* p_err_message = nullptr;

	// VRAM 上に確保するメモリやリソースの利用方法などを設定する型
	D3D11_BUFFER_DESC buff_desc;
	D3D11_SUBRESOURCE_DATA sub_resouce_data;

	// DxLib から DirectX デバイスを取得
	// DirectX デバイスが DirectX 本体で各種必要なデータの生成などを行います
	ID3D11Device* pd3dDevice = (ID3D11Device*)DxLib::GetUseDirect3D11Device();

	//-------------------------------------------------------------------------------------------------
	// 
	// 頂点バッファの作成
	// 
	ID3D11Buffer* p_vertex_buffer = nullptr;
	ZeroMemory(&buff_desc, sizeof(buff_desc));
	buff_desc.Usage = D3D11_USAGE_DEFAULT;
	buff_desc.ByteWidth = (UINT)(sizeof(TEST_VERTEX) * MESH_VERTEX_NUM);
	buff_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buff_desc.CPUAccessFlags = 0;
	ZeroMemory(&sub_resouce_data, sizeof(sub_resouce_data));
	sub_resouce_data.pSysMem = mesh_vertexs;
	hr = pd3dDevice->CreateBuffer(&buff_desc, &sub_resouce_data, &p_vertex_buffer);
	if (S_OK != hr) {
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		tnl::DebugTrace("Error : 頂点バッファの生成に失敗 \n");
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		return;
	}
	vertex_buffer.Attach(p_vertex_buffer); // ComPtr に受け取り


	//-------------------------------------------------------------------------------------------------
	// 
	// インデックスバッファの作成
	//
	ID3D11Buffer* p_index_buffer = nullptr;
	buff_desc.Usage = D3D11_USAGE_DEFAULT;
	buff_desc.ByteWidth = sizeof(UINT) * INDEX_NUM;
	buff_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buff_desc.CPUAccessFlags = 0;
	buff_desc.MiscFlags = 0;
	sub_resouce_data.pSysMem = mesh_vtx_indexs;
	hr = pd3dDevice->CreateBuffer(&buff_desc, &sub_resouce_data, &p_index_buffer);
	if (S_OK != hr) {
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		tnl::DebugTrace("Error : インデックスデータの作成に失敗 \n");
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		return;
	}
	index_buffer.Attach(p_index_buffer);


	//-------------------------------------------------------------------------------------------------
	// 
	// 頂点シェーダの作成
	//
	ID3DBlob* p_blob_vertex = nullptr;
	hr = D3DCompileFromFile(L"program/shader/example/simple_test.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"VS_Main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &p_blob_vertex, &p_err_message);
	if (S_OK != hr) {
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		tnl::DebugTrace("Error : 頂点シェーダのコンパイルに失敗 \n");
		OutputDebugStringA(static_cast<char*>(p_err_message->GetBufferPointer()));
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		return;
	}

	ID3D11VertexShader* p_vertex_shader = nullptr;
	hr = pd3dDevice->CreateVertexShader(p_blob_vertex->GetBufferPointer(), p_blob_vertex->GetBufferSize(), NULL, &p_vertex_shader);
	vertex_shader.Attach(p_vertex_shader);
	if (S_OK != hr) {
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		tnl::DebugTrace("Error : 頂点シェーダの作成に失敗 \n");
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		return;
	}


	//-------------------------------------------------------------------------------------------------
	//
	// ピクセルシェーダの作成
	//
	ID3DBlob* p_blob_pixel = nullptr;
	hr = D3DCompileFromFile(L"program/shader/example/simple_test.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"PS_Main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &p_blob_pixel, &p_err_message);
	if (S_OK != hr) {
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		tnl::DebugTrace("Error : ピクセルシェーダのコンパイルに失敗 \n");
		OutputDebugStringA(static_cast<char*>(p_err_message->GetBufferPointer()));
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		return;
	}

	ID3D11PixelShader* p_pixel_shader = nullptr;
	hr = pd3dDevice->CreatePixelShader(p_blob_pixel->GetBufferPointer(), p_blob_pixel->GetBufferSize(), NULL, &p_pixel_shader);
	pixel_shader.Attach(p_pixel_shader);
	if (S_OK != hr) {
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		tnl::DebugTrace("Error : ピクセルシェーダの作成に失敗 \n");
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		return;
	}


	//-------------------------------------------------------------------------------------------------
	// 
	// 頂点インプットレイアウト作成
	//
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},   // 
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},  // 
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},  // 
	};

	ID3D11InputLayout* il = nullptr;
	hr = pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout),
		p_blob_vertex->GetBufferPointer(), p_blob_vertex->GetBufferSize(),
		&il);
	input_layout.Attach(il);
	if (S_OK != hr) {
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		tnl::DebugTrace("Error : 頂点インプットレイアウトの作成に失敗 \n");
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		return;
	}


	//-------------------------------------------------------------------------------------------------
	// 
	// 定数バッファの作成
	//
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.ByteWidth = sizeof(TEST_CBUFFER);
	ID3D11Buffer* cb = nullptr;
	hr = pd3dDevice->CreateBuffer(&bd, nullptr, &cb);
	constant_buffer.Attach(cb);
	if (S_OK != hr) {
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		tnl::DebugTrace("Error : 定数バッファの作成に失敗 \n");
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		return;
	}


	//-------------------------------------------------------------------------------------------------
	// 
	// テクスチャからGPU用シェーダリソースを作成
	//
	texture = dxe::Texture::CreateFromFile("resource/graphics/example/test.jpg");

	ID3D11Resource* res = nullptr;
	ID3D11ShaderResourceView* srv = nullptr;
	//wchar_t buff[256] = { 0 };
	//tnl::ToWChara(buff, texture_file_path);
	//hr = CreateWICTextureFromFile(pd3dDevice, buff, &res, &srv);
	hr = CreateWICTextureFromMemory(pd3dDevice, (uint8_t*)texture->getFileData(), (size_t)texture->getDataSize(), &res, &srv);
	tex_res.Attach(res);
	tex_srv.Attach(srv);
	if (S_OK != hr) {
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		tnl::DebugTrace("Error : シェーダリソースの作成に失敗 \n");
		tnl::DebugTrace("-----------------------------------------------------------------\n");
		return;
	}


}


//------------------------------------------------------------------------------------------------------------
// 毎フレーム実行されます
void gameMain(float delta_time) {


	rotation *= tnl::Quaternion::RotationAxis({ 0, 1, 0 }, tnl::ToRadian(1));


	HRESULT hr = E_FAIL;

	// DxLib から DirectX デバイスコンテキストを取得
	// デバイスコンテキストが描画命令などを行います
	ID3D11DeviceContext* pImmediateContext = (ID3D11DeviceContext*)DxLib::GetUseDirect3D11DeviceContext();


	// カメラ更新
	camera->update();


	// DxLib の各種設定を保存
	ID3D11VertexShader* bk_vs = nullptr;
	ID3D11PixelShader* bk_ps = nullptr;
	ID3D11ShaderResourceView* bk_srv = nullptr;
	pImmediateContext->VSGetShader(&bk_vs, NULL, 0);
	pImmediateContext->PSGetShader(&bk_ps, NULL, 0);
	pImmediateContext->PSGetShaderResources(0, 1, &bk_srv);


	//-------------------------------------------------------------------------------------------
	// 
	// 定数バッファに CPU 側で計算したデータを転送
	//
	{
		TEST_CBUFFER* cbuffer = nullptr;
		XMMATRIX matView, matProj, matWorld;
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		matProj = XMLoadFloat4x4(&camera->getProjectionMatrix());
		matView = XMLoadFloat4x4(&camera->getViewMatrix());

		tnl::Matrix affine = tnl::Matrix::AffineTransformation(position, scale, rotation);
		matWorld = XMLoadFloat4x4(&affine);

		ID3D11Buffer* cb = constant_buffer.Get();
		hr = pImmediateContext->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		cbuffer = (TEST_CBUFFER*)(mappedResource.pData);

		cbuffer->mat_view_ = matView;
		cbuffer->mat_proj_ = matProj;
		cbuffer->mat_world_ = matWorld;

		pImmediateContext->Unmap(cb, 0);

		// 定数バッファを頂点シェーダにセット
		pImmediateContext->VSSetConstantBuffers(0, 1, &cb);

	}

	// プリミティブ タイプおよびデータの順序に関する情報を設定
	pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 深度ステンシルバッファの設定
	pImmediateContext->OMSetDepthStencilState(dxe::GetDepthStencilState(dxe::eDepthStenclil::DEPTH_W_ON_T_ON), 0);

	// ラスタライザステート設定
	pImmediateContext->RSSetState(dxe::GetRasterizerState(dxe::eRasterizerState::CULL_NONE));

	// ブレンドステート設定
	pImmediateContext->OMSetBlendState(dxe::GetBlendState(dxe::eBlendState::NORMAL), 0, 0xffffffff);

	// サンプラステートを設定する
	ID3D11SamplerState* st = dxe::GetSamplerState(dxe::eSamplerState::ANISOTROPIC);
	pImmediateContext->PSSetSamplers(0, 1, &st);


	// 入力アセンブラー ステージに頂点バッファーの配列をバインドする。
	UINT stride = sizeof(TEST_VERTEX);
	UINT offset = 0;
	ID3D11Buffer* vb = vertex_buffer.Get();
	pImmediateContext->IASetVertexBuffers(0, 1, &vb, &stride, &offset);


	//インデックスバッファのセット
	pImmediateContext->IASetIndexBuffer(index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// 頂点データレイアウト設定
	pImmediateContext->IASetInputLayout(input_layout.Get());

	// 頂点シェーダーをセット
	pImmediateContext->VSSetShader(vertex_shader.Get(), NULL, 0);

	// ピクセルシェーダーをセット
	pImmediateContext->PSSetShader(pixel_shader.Get(), NULL, 0);

	// テクスチャーを設定する
	ID3D11ShaderResourceView* srv = tex_srv.Get();
	pImmediateContext->PSSetShaderResources(0, 1, &srv);

	// 描画命令
	pImmediateContext->DrawIndexed(INDEX_NUM, 0, 0);


	// 各種設定を DxLib に戻す
	pImmediateContext->VSSetShader(bk_vs, NULL, 0);
	pImmediateContext->PSSetShader(bk_ps, NULL, 0);
	pImmediateContext->PSSetShaderResources(0, 1, &bk_srv);


	dxe::DrawFpsIndicator({ 10, DXE_WINDOW_HEIGHT - 10 });
}

//------------------------------------------------------------------------------------------------------------
// ゲーム終了時に１度だけ実行されます
void gameEnd() {

}

#endif