#include "Particle.hlsli"

static const int vnum = 4;

static const float4 offset_array[vnum] =
{
    float4(-0.2f,  0.2f, 0.0f, 0.0f), // ����
	float4( 0.2f,  0.2f, 0.0f, 0.0f), // �E��
	float4(-0.2f, -0.2f, 0.0f, 0.0f), // ����
	float4( 0.2f, -0.2f, 0.0f, 0.0f), // �E��

};

[maxvertexcount(vnum)]
void main(
	point PS_INPUT input[1],
	inout TriangleStream<PS_INPUT> output
)
{
    // ���S�_�����[���h��Ԃɕϊ�
    float4 center = mul(input[0].positionCS, m_worldMatrix);
    
    PS_INPUT element = (PS_INPUT)0;

    for (int i = 0; i < vnum; ++i)
    {
        // ��{�ʒu
        element.positionCS = center;
        
        // �e���_�̃��[�J���I�t�Z�b�g�����Z
        element.positionCS += mul(offset_array[i], m_billboardMatrix);
        
        // �r���[�E�v���W�F�N�V�����ϊ�
        //element.positionCS = mul(element.positionCS, m_billboardMatrix);
        element.positionCS = mul(element.positionCS, m_viewMatrix);
        element.positionCS = mul(element.positionCS, m_projectionMatrix);
        
        // �F���擾
        element.color = input[0].color;
        
        element.uv = (offset_array[i].xy + 0.5f);
        
        // ���_���o�̓X�g���[���ɒǉ�
        output.Append(element);
    }
    
    // �|���S���`��̋�؂�
    output.RestartStrip();
}