using System.Collections;
using JellyBitEngine;

public class Ambient : JellyScript
{
    //public GameObject alitaGO = null;
    //private Alita alita = null;

    public static Ambient Instance;
    public bool lowlife = false;
    public bool diying = false;
    public bool dead = false;

    private AudioSource audioSource = null;

    //Use this method for initialization
    public override void Awake()
    {
        //alita = alitaGO.GetComponent<Alita>();
        Instance = this;
        audioSource = gameObject.GetComponent<AudioSource>();
    }

    //Called every frame
    public override void Update()
    {
        if (audioSource != null)
        {
            if (lowlife && !diying)
            {
                ModuleAudio.Instance.CleanAudio(audioSource);
                ModuleAudio.Instance.PlayOnce(audioSource, ModuleAudio.Instance.lowLife);
                lowlife = false;
                diying = true;
            }
            if(dead)
            {
                ModuleAudio.Instance.CleanAudio(audioSource);
                lowlife = false;
                diying = false;
                dead = false;
            }
        }
    }
}

