struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

cbuffer CameraParams : register(b0)
{
    float p_fieldOfView;
    float p_nearClip;
    float p_farClip;
    float p_aspectRatio;
    float3 p_cameraPosition;
    float4x4 p_viewMat;
    uint p_numBounces;
};

//TextureCube t_skybox : register(t0);
//SamplerState s_skyboxSampler : register(s0);

struct MeshData
{
    uint indexCount;
    uint vertexCount;
    float roughness;
    float metallic;
    float4 color;
};

RWStructuredBuffer<MeshData> u_meshes : register(u0);
RWBuffer<uint> u_indices : register(u1);

struct VertexData
{
    float3 position;
    float3 normal;
    float2 uv;
    float3 tangent;
    float3 bitangent;
};

RWStructuredBuffer<VertexData> u_vertices : register(u2);

struct Ray
{
    float3 origin;
    float3 dir;
};

struct Triangle
{
    VertexData vertA;
    VertexData vertB;
    VertexData vertC;
};

struct HitInfo
{
    bool hit;
    float3 hitPoint;
    float t;
    float3 normal;
    float2 uv;
    float4 color;
    float roughness;
    float metallic;
};

HitInfo RayTriangle(Ray ray, Triangle tri)
{
    float3 edgeAB = tri.vertB.position - tri.vertA.position;
    float3 edgeAC = tri.vertC.position - tri.vertA.position;
    float3 normalVector = cross(edgeAB, edgeAC);
    float3 ao = ray.origin - tri.vertA.position;
    float3 dao = cross(ao, ray.dir);

    float determinant = -dot(ray.dir, normalVector);
    float invDet = 1 / determinant;
    
    float dst = dot(ao, normalVector) * invDet;
    float u = dot(edgeAC, dao) * invDet;
    float v = -dot(edgeAB, dao) * invDet;
    float w = 1 - u - v;
    
    HitInfo hitInfo;
    
    hitInfo.hit = determinant >= 1E-6 && dst >= 0 && u >= 0 && v >= 0 && w >= 0;
    hitInfo.hitPoint = ray.origin + ray.dir * dst;
    hitInfo.t = dst;

    hitInfo.normal = normalize(tri.vertA.normal * w + tri.vertB.normal * u + tri.vertC.normal * v);
    hitInfo.uv = tri.vertA.uv * w + tri.vertB.uv * u + tri.vertC.uv * v;
    
    return hitInfo;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    uint meshCount;
    uint meshDataStride;
    u_meshes.GetDimensions(meshCount, meshDataStride);
    
    float height = p_nearClip * tan(radians(p_fieldOfView * .5f)) * 2;
    float width = height * p_aspectRatio;
    
    float3 viewPoint = mul(p_viewMat, float4(float3(input.uv - .5, 1) * float3(width, -height, p_nearClip), 1));

    Ray ray;
    ray.origin = p_cameraPosition;
    ray.dir = normalize(viewPoint - ray.origin);
    
    float3 ouputColor = float3(1, 1, 1);
    
    for (uint i = 0; i < 1 + p_numBounces; i++)
    {
        uint indexBufferOffset = 0;
        uint vertexBufferOffset = 0;
    
        float closestT = p_farClip;
        HitInfo closestHit;
        closestHit.hit = false;
    
        for (uint meshI = 0; meshI < meshCount; meshI++)
        {
            MeshData meshData = u_meshes.Load(meshI);

            for (uint indexI = 0; indexI < meshData.indexCount; indexI += 3)
            {
                Triangle tri;
            
                VertexData vertA = u_vertices.Load(u_indices.Load(indexI));
                VertexData vertB = u_vertices.Load(u_indices.Load(indexI + 1));
                VertexData vertC = u_vertices.Load(u_indices.Load(indexI + 2));
            
                tri.vertA = vertA;
                tri.vertB = vertB;
                tri.vertC = vertC;
            
                HitInfo hitInfo = RayTriangle(ray, tri);

                if (hitInfo.hit && hitInfo.t < closestT)
                {
                    hitInfo.color = meshData.color;

                    if (hitInfo.color.a <= 0)
                    {
                        continue;
                    }
                
                    hitInfo.roughness = meshData.roughness;
                    hitInfo.metallic = meshData.metallic;
                
                    closestT = hitInfo.t;
                    closestHit = hitInfo;
                }
            }
        
            indexBufferOffset += meshData.indexCount;
            vertexBufferOffset += meshData.vertexCount;
        }
        
        if (closestHit.hit)
        {
            ouputColor = closestHit.color * max(dot(normalize(float3(-1, 1, -1)), closestHit.normal), .1);

        }
    }

    return float4(ouputColor, 1);
}