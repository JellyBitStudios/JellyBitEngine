using System.Collections;
using JellyBitEngine;


class BulletMovement : JellyScript
{
    public GameObject direction;
    public float speed = 25f;
    public float life = 10f;
    public bool isInmortal = true;

    private float lifeTime = 0f;
    private Vector3 dir = new Vector3(0, 0, 0);
    private bool getDirection = true;

    //Life and dmg
    public int damage = 10;
    private int current_life = 100;

    //Audio
    private AudioSource audioSource = null;

    public override void Awake()
    {
        lifeTime = Time.time;
        audioSource = direction.GetComponent<AudioSource>();
    }

    public override void Update()
    {
        if (getDirection)
        {
            dir = (direction.transform.position - transform.position).normalized();

            getDirection = false;
        }

        transform.position += dir * (speed * Time.deltaTime);

        if (Time.time > lifeTime + life)
        {
            if (!isInmortal)
                Destroy(gameObject);
        }
    }

    public void Hit(int damage)
    {
        current_life -= damage;

        Debug.Log("Hostia de bola");

        //Supongo que esta parte esta en proceso, no entiendo que hacéis con el current life. De quén és de Alita? de la bola?
        //Cuando haga cosas avisadme: @oriol de dios
        //
        //ModuleAudio.Instance.CleanAudio(audioSource);
        //ModuleAudio.Instance.ResetIdleCounter();
        //ModuleAudio.Instance.PlayHittedActor(audioSource, unit_type, current_life, 10.0f, max_life, ModuleAudio.Instance.hitScream_1,
        //                                                                                    ModuleAudio.Instance.hitScream_2,
        //                                                                                    ModuleAudio.Instance.hitScream_3);


        if (current_life <= 0)
            if (!isInmortal)
                Destroy(gameObject);
    }

}