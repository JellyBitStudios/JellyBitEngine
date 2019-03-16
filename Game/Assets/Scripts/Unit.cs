using System.Collections;
using JellyBitEngine;

public class Unit : JellyScript
{
    // General Stats
    public float damage = 25;
    public float mov_speed = 1.0f;

    // Life
    public float current_life = 100;
    public float max_life = 100;

    //Hit particles
    public GameObject particleHit = null;
    private bool hitted = false;
    private float particle_time = 0.0f;
    public float particle_life = 0.60f;

    //Audio Source
    private AudioSource audioSource = null;

    public GameObject fake_dead = null;

    public enum Unit_Type
    {
        NONE,
        ALITA,
        ENEMY
    }
    public Unit_Type unit_type = Unit_Type.NONE;

    // Start is called before the first frame update
    public override void Start()
    {
        audioSource = gameObject.GetComponent<AudioSource>();
    }

    public override void Update()
    {
        if (hitted)
        {
            particle_time += Time.deltaTime;

            if (particle_time >= particle_life && particleHit != null)
            {
                particleHit.GetComponent<ParticleEmitter>().Stop();
                particle_time = 0.0f;
            }
        }
    }

    public void Hit(int damage)
    {
        current_life -= damage;

        if (particleHit != null && hitted == false)
        {
            particleHit.GetComponent<ParticleEmitter>().Play();
            hitted = true;
        }

        if (current_life <= 0)
        {
            switch (unit_type)
            {
                case Unit_Type.ALITA:
                    current_life = max_life;
                    Debug.Log("SEÑOR STARK NO ME ENCUENTRO MUY BIEN...");
                    ModuleAudio.Instance.CanPlayRunning();
                    ModuleAudio.Instance.ResetIdleCounter();
                    ModuleAudio.Instance.CleanAudio(audioSource);
                    ModuleAudio.Instance.PlayDiyingFX(audioSource);
                    break;
                case Unit_Type.ENEMY:
                    if(fake_dead != null)
                        GameObject.Instantiate(fake_dead, gameObject.transform.position,gameObject.transform.rotation);
                    Destroy(gameObject);
                    break;
                case Unit_Type.NONE:
                    Debug.Log("There is type of unit to damage!!!");
                    break;
            }
        }
        else
        {
            switch (unit_type)
            {
                case Unit_Type.ALITA:
                    Debug.Log("AUCH");
                    //ModuleAudio.Instance.CleanAudio(audioSource);
                    ModuleAudio.Instance.CanPlayRunning();
                    ModuleAudio.Instance.ResetIdleCounter();
                    ModuleAudio.Instance.CleanAudio(audioSource);
                    ModuleAudio.Instance.PlayHittedActor(audioSource,unit_type,current_life,max_life,ModuleAudio.Instance.auchAlita_1,
                                                                                                        ModuleAudio.Instance.auchAlita_2,
                                                                                                        ModuleAudio.Instance.auchAlita_3, 30.0f);
                    break;
                case Unit_Type.ENEMY:
                    //play auchrobot
                    break;
                case Unit_Type.NONE:
                    Debug.Log("There is type of unit to kill!!!");
                    break;
            }
        }
    }

}
