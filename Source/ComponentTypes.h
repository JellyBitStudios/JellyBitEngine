#ifndef COMPONENT_TYPES_H
#define COMPONENT_TYPES_H

enum ComponentTypes
{
	NoComponentType,

	TransformComponent,
	MeshComponent,
	MaterialComponent,
	CameraComponent,
	NavAgentComponent,
	EmitterComponent,
	ScriptComponent,
	BoneComponent,
	AnimationComponent,
	AnimatorComponent,
	LightComponent = 16,
	ProjectorComponent,

	//UI
	CanvasComponent = 32,
	RectTransformComponent = 11,
	CanvasRendererComponent,
	ImageComponent,
	ButtonComponent,
	LabelComponent,

	// Physics
	/// Rigid Actors
	RigidStaticComponent = 18,
	RigidDynamicComponent,

	/// Colliders
	BoxColliderComponent,
	SphereColliderComponent,
	CapsuleColliderComponent,
	PlaneColliderComponent,

	/// Joints
	FixedJointComponent,
	DistanceJointComponent,
	SphericalJointComponent,
	RevoluteJointComponent,
	PrismaticJointComponent,
	D6JointComponent,
	//Audio
	AudioListenerComponent,
	AudioSourceComponent //31

	//32 Canvas, defined before.
};

#endif