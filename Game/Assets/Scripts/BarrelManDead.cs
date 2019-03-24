using System.Collections;
using JellyBitEngine;
using System;



class BarrelManDead : JellyScript
{
    public float time_to_destroy = 1.0f;
    private float actual_time = 0.0f;

    public GameObject sphere_center = null;
    public float radius = 1.0f;
    public LayerMask mask = new LayerMask();
    public int damage = 30;

    public override void Awake()
    {
        if (gameObject.GetComponent<ParticleEmitter>() != null)
            gameObject.GetComponent<ParticleEmitter>().Play();
    }

    public override void Update()
    {
        if (actual_time > time_to_destroy)
        {
            OverlapHit[] hits;
            if (Physics.OverlapSphere(radius, sphere_center.transform.position, out hits, mask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                foreach (OverlapHit hit in hits)
                {
                    if (hit.gameObject.GetComponent<Unit>() != null)
                    {
                        hit.gameObject.GetComponent<Unit>().Hit(damage);
                    }

                }
            }

            Destroy(gameObject);
        }
        else
            actual_time += Time.deltaTime;
    }

}

