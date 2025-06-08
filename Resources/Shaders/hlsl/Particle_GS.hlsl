#include "Particle.hlsli"

static const int vnum = 4;

static const float4 offset_array[vnum] =
{
    float4(-0.2f,  0.2f, 0.0f, 0.0f), // 左上
	float4( 0.2f,  0.2f, 0.0f, 0.0f), // 右上
	float4(-0.2f, -0.2f, 0.0f, 0.0f), // 左下
	float4( 0.2f, -0.2f, 0.0f, 0.0f), // 右下

};

[maxvertexcount(vnum)]
void main(
	point PS_INPUT input[1],
	inout TriangleStream<PS_INPUT> output
)
{
    // 中心点をワールド空間に変換
    float4 center = mul(input[0].positionCS, m_worldMatrix);
    
    PS_INPUT element = (PS_INPUT)0;

    for (int i = 0; i < vnum; ++i)
    {
        // 基本位置
        element.positionCS = center;
        
        // 各頂点のローカルオフセットを加算
        element.positionCS += mul(offset_array[i], m_billboardMatrix);
        
        // ビュー・プロジェクション変換
        //element.positionCS = mul(element.positionCS, m_billboardMatrix);
        element.positionCS = mul(element.positionCS, m_viewMatrix);
        element.positionCS = mul(element.positionCS, m_projectionMatrix);
        
        // 色を取得
        element.color = input[0].color;
        
        element.uv = (offset_array[i].xy + 0.5f);
        
        // 頂点を出力ストリームに追加
        output.Append(element);
    }
    
    // ポリゴン描画の区切り
    output.RestartStrip();
}