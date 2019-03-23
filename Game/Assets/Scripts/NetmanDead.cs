using System.Collections;
using JellyBitEngine;
using System;


class NetmanDead : JellyScript
{
    public float time_to_destroy = 1.0f;
    private float actual_time = 0.0f;

    public override void Awake()
    {
        if(gameObject.GetComponent<ParticleEmitter>() != null)
            gameObject.GetComponent<ParticleEmitter>().Play();
    }

    public override void Update()
    {
        if (actual_time > time_to_destroy)
        {
            Destroy(gameObject);
        }
        else
            actual_time += Time.deltaTime;
    }
}

