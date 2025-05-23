

struct MatrixPair
{
    float4x4 Matrix1;
    float4x4 Matrix2;
};

StructuredBuffer<MatrixPair> Input : register(t0);

// �o�́F�|���Z����1�̍s��
RWStructuredBuffer<float4x4> Output : register(u0);

[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{ 
    
    // �͈̓`�F�b�N��ǉ��F�r���[���傫���Ă��A�N�Z�X���Ȃ��悤�ɂ���
    if (DTid.x >= 10000)
        return;
    
    uint i = DTid.x;
    Output[i] = mul(Input[i].Matrix1, Input[i].Matrix2);
    
}