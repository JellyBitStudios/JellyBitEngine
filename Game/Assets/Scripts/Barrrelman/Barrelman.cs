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
        if(Input.GetKeyDown(KeyCode.KEY_1))
            animator.AnimationFinished();
    }
}

