using System.Collections;
using JellyBitEngine;

public class alita_thing : JellyScript
{
    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        if (Input.GetKeyDown(KeyCode.KEY_1))
        {
            gameObject.GetComponent<Animator>().PlayAnimation("anim_run_alita_fist");
        }
        else if (Input.GetKeyDown(KeyCode.KEY_2))
            gameObject.GetComponent<Animator>().PlayAnimation("anim_special_attack_q_alita_fist");
    }
}

