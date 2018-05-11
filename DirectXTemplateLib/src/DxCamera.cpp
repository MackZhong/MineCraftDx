#include "DirectXTemplateLibPCH.h"
#include "DxCamera.h""

using namespace DirectX;

DxCamera::DxCamera( Handedness handedness )
    : m_ViewDirty( true )
    , m_InverseViewDirty( true )
    , m_ProjectionDirty( true )
    , m_InverseProjectionDirty( true )
    , m_Handedness( handedness )
    , m_vFoV( 45.0f )
    , m_AspectRatio( 1.0f )
    , m_zNear( 0.1f )
    , m_zFar( 100.0f )
{
    pData = (AlignedData*)_aligned_malloc( sizeof(AlignedData), 16 );
    if ( pData == NULL )
    {
        MessageBoxW( nullptr, L"The data is NULL?!", L"Error", MB_OK|MB_ICONERROR );
    }
    pData->m_Translation = XMVectorZero();
    pData->m_Rotation = XMQuaternionIdentity();
}

DxCamera::~DxCamera()
{
    _aligned_free(pData);
}

void DxCamera::set_Viewport( D3D11_VIEWPORT viewport )
{
    m_Viewport = viewport;
}

D3D11_VIEWPORT DxCamera::get_Viewport() const
{
    return m_Viewport;
}

void XM_CALLCONV DxCamera::set_LookAt( FXMVECTOR eye, FXMVECTOR target, FXMVECTOR up )
{
    switch ( m_Handedness )
    {
    case LeftHanded:
        {
            pData->m_ViewMatrix = XMMatrixLookAtLH( eye, target, up );
        }
        break;
    case RightHanded:
        {
            pData->m_ViewMatrix = XMMatrixLookAtRH( eye, target, up );
        }
        break;
    }

    pData->m_Translation = eye;
    pData->m_Rotation = XMQuaternionRotationMatrix( XMMatrixTranspose(pData->m_ViewMatrix) );

    m_InverseViewDirty = true;
    m_ViewDirty = false;
}

XMMATRIX DxCamera::get_ViewMatrix() const
{
    if ( m_ViewDirty )
    {
        UpdateViewMatrix();
    }
    return pData->m_ViewMatrix;
}

XMMATRIX DxCamera::get_InverseViewMatrix() const
{
    if ( m_InverseViewDirty )
    {
        pData->m_InverseViewMatrix = XMMatrixInverse( nullptr, pData->m_ViewMatrix );
        m_InverseViewDirty = false;
    }

    return pData->m_InverseViewMatrix;
}

void DxCamera::set_Projection( float fovy, float aspect, float zNear, float zFar )
{
    m_vFoV = fovy;
    m_AspectRatio = aspect;
    m_zNear = zNear;
    m_zFar = zFar;

    m_ProjectionDirty = true;
    m_InverseProjectionDirty = true;
}

XMMATRIX DxCamera::get_ProjectionMatrix() const
{
    if ( m_ProjectionDirty )
    {
        UpdateProjectionMatrix();
    }

    return pData->m_ProjectionMatrix;
}

XMMATRIX DxCamera::get_InverseProjectionMatrix() const
{
    if ( m_InverseProjectionDirty )
    {
        UpdateInverseProjectionMatrix();
    }

    return pData->m_InverseProjectionMatrix;
}

void DxCamera::set_Translation( FXMVECTOR translation )
{
    pData->m_Translation = translation;
    m_ViewDirty = true;
}

XMVECTOR DxCamera::get_Translation() const
{
    return pData->m_Translation;
}

void DxCamera::set_Rotation( FXMVECTOR rotation )
{
    pData->m_Rotation = rotation;
}

XMVECTOR DxCamera::get_Rotation() const
{
    return pData->m_Rotation;
}

void XM_CALLCONV DxCamera::Translate( FXMVECTOR translation, Space space )
{
    switch ( space )
    {
    case LocalSpace:
        {
            pData->m_Translation += XMVector3Rotate( translation, pData->m_Rotation );
        }
        break;
    case WorldSpace:
        {
            pData->m_Translation += translation;
        }
        break;
    }

    pData->m_Translation = XMVectorSetW( pData->m_Translation, 1.0f );

    m_ViewDirty = true;
    m_InverseViewDirty = true;
}

void DxCamera::Rotate( FXMVECTOR quaternion )
{
    pData->m_Rotation = XMQuaternionMultiply( pData->m_Rotation, quaternion );

    m_ViewDirty = true;
    m_InverseViewDirty = true;
}

void DxCamera::UpdateViewMatrix() const
{
    XMMATRIX rotationMatrix = XMMatrixTranspose(XMMatrixRotationQuaternion( pData->m_Rotation ));
    XMMATRIX translationMatrix = XMMatrixTranslationFromVector( -(pData->m_Translation) );

    pData->m_ViewMatrix = translationMatrix * rotationMatrix;

    m_InverseViewDirty = true;
    m_ViewDirty = false;
}

void DxCamera::UpdateInverseViewMatrix() const
{
    if ( m_ViewDirty )
    {
        UpdateViewMatrix();
    }

    pData->m_InverseViewMatrix = XMMatrixInverse( nullptr, pData->m_ViewMatrix );
    m_InverseViewDirty = false;
}

void DxCamera::UpdateProjectionMatrix() const
{
    switch( m_Handedness )
    {
    case LeftHanded:
        {
            pData->m_ProjectionMatrix = XMMatrixPerspectiveFovLH( XMConvertToRadians(m_vFoV), m_AspectRatio, m_zNear, m_zFar );
        }
        break;
    case RightHanded:
        {
            pData->m_ProjectionMatrix = XMMatrixPerspectiveFovRH( XMConvertToRadians(m_vFoV), m_AspectRatio, m_zNear, m_zFar );
        }
        break;
    }

    m_ProjectionDirty = false;
    m_InverseProjectionDirty = true;
}

void DxCamera::UpdateInverseProjectionMatrix() const
{
    if ( m_ProjectionDirty )
    {
        UpdateProjectionMatrix();
    }

    pData->m_InverseProjectionMatrix = XMMatrixInverse( nullptr, pData->m_ProjectionMatrix );
    m_InverseProjectionDirty = false;
}
