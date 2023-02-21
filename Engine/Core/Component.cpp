#include "stdafx.h"
#include "Component.h"

#include "SceneObject.h"

Component::Component(SceneObject* owner) : Object(owner->name)
{
    this->owner = owner;
    this->name = owner->name + " : " + typeid(this).name();
}

SceneObject* Component::GetOwner()
{
    return owner;
}

Transform* Component::GetTransform()
{
    return GetOwner()->GetTransform();
}

void Component::SetTransform(Transform* transform)
{
    GetOwner()->SetTransform(transform);
}

void Component::OnStart()
{

}

void Component::OnUpdate()
{

}

void Component::OnEnd()
{

}
