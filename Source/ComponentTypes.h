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
	ScriptComponent = 7,
	BoneComponent,
	AnimationComponent,
	AnimatorComponent,
	LightComponent = 16,
	ProjectorComponent,

	// Effects
	EmitterComponent = 6,
	TrailComponent = 33,

	//UI
	CanvasComponent = 32,
	RectTransformComponent = 11,
	CanvasRendererComponent,
	ImageComponent,
	ButtonComponent,
	LabelComponent,
	SliderComponent = 34,
	UIAnimationComponent,

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
	AudioSourceComponent, //31

	// CanvasComponent = 32
	// TrailComponent = 33
	// SliderComponent = 34
	// UIAnimatorComponent = 35

	//msg: the before elements registred is updated. It's fine this margin :ok_hand:
	InterpolationComponent = 40 // I'm not sure if this can interfere with other components, so I've left a margin of safety
};

#endif
