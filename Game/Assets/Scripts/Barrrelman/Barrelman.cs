using JellyBitEngine;

class Barrelman : JellyScript
{

    public Animator animator;

    public override void Awake()
    {
        animator = gameObject.GetComponent<Animator>();
    }

    public override void Update()
    {
        if (Input.GetKeyDown(KeyCode.KEY_0))
        {
            animator.UpdateAnimationBlendTime(0.0f);
            animator.PlayAnimation("barrelman_idle_anim_z");
        }
        if (Input.GetKeyDown(KeyCode.KEY_1))
        {
            animator.PlayAnimation("barrelman_walk_anim_z");
        }
        if (Input.GetKeyDown(KeyCode.KEY_2))
        {
            animator.PlayAnimation("barrelman_attack_anim_z");
            animator.SetAnimationLoop(false);
        }
    }
}

