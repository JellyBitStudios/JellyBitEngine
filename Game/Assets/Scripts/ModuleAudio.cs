using System.Collections;
using System;
using JellyBitEngine;

public class ModuleAudio : JellyScript
{
    public static ModuleAudio Instance;
    public float waitingIdleSound = 5.0f;
    public bool canPlayRunning = true;
    public bool dashPlayed = false;
    public float idleSoundTimer = 0.0f;

    private int lastRand = 0;

    //Alita FX sounds
    public readonly string running = "Alita_running";
    public readonly string lowLife = "Alita_lowlife";
    public readonly string idle_1 = "Alita_idle_1";
    public readonly string idle_2 = "Alita_idle_2";
    public readonly string idle_3 = "Alita_idle_3";
    public readonly string areaAttackFX = "Alita_skill";
    public readonly string areaAttackScreamFX = "Alita_skill_scream";
    public readonly string dashFX = "Alita_dash";
    public readonly string hitScream_1 = "Alita_attack_scream_1";
    public readonly string hitScream_2 = "Alita_attack_scream_2";
    public readonly string hitScream_3 = "Alita_attack_scream_3";
    public readonly string dieSound = "Alita_diying_ambient";
    public readonly string dieScream = "Alita_diying_scream";
    public readonly string auchAlita_1 = "Alita_hitted_1";
    public readonly string auchAlita_2 = "Alita_hitted_2";
    public readonly string auchAlita_3 = "Alita_hitted_3";


    //Use this method for initialization
    public override void Awake()
    {
        Instance = this;
    }

    //Called every frame
    public override void Update()
    {

    }

    //Sound functions---------------------------------------

    public void ResetIdleCounter() { idleSoundTimer = 0.0f; }

    public void CanPlayRunning() { canPlayRunning = true; }

    public void CleanAudio(AudioSource audioSource)
    {
        //if (!canPlayRunning)
        
        audioSource.StopAudio();
        Debug.Log("Alita STFU!");
        canPlayRunning = true;
        
    }

    public void PlayRandomeSound(AudioSource audioSource, string sound1, string sound2, string sound3)
    {
        int rand = 0;
        Random r = new Random();
        do
        {
            rand = r.Next(1, 4);
        } while (lastRand == rand);

        lastRand = rand;

        switch (rand)
        {
            case 1:
                if (sound1 != null)
                {
                    audioSource.audio = sound1;
                    Debug.Log(sound1);
                }
                break;
            case 2:
                if (sound2 != null)
                {
                    audioSource.audio = sound2;
                    Debug.Log(sound2);
                }
                break;
            case 3:
                if (sound3 != null)
                {
                    audioSource.audio = sound3;
                    Debug.Log(sound3);
                }
                break;
        }
        audioSource.PlayAudio();
    }

    public void PlayOnce(AudioSource audioSource, string sound)
    {
        if (sound != null && audioSource != null)
        {
            audioSource.audio = sound;
            audioSource.PlayAudio();
        }
        else
        {
            Debug.Log("ALITA'S AUDIO SOURCE MISSING");
        }
    }

    //ALITA ----------------------------------------------------
    public void PlayRunningFX(AudioSource audioSource)
    {
        //FX
        if (audioSource != null)
        {
            if (running != null)
            {
                audioSource.audio = running;
                audioSource.loop = true;
                audioSource.PlayAudio();
            }
        }
        else
        {
            Debug.Log("ALITA'S AUDIO SOURCE MISSING");
        }
    }

    public void PlayIdleFX(AudioSource audioSource)
    {
        idleSoundTimer += Time.deltaTime;
        //FX
        if (audioSource != null)
        {
            if (idleSoundTimer > waitingIdleSound)
            {
                PlayRandomeSound(audioSource, idle_1, idle_2, idle_3);
                ResetIdleCounter();
            }
        }
        else
        {
            Debug.Log("ALITA'S AUDIO SOURCE MISSING");
        }
    }

    public void PlayDashFX(AudioSource audioSource)
    {
        //FX
        if (audioSource != null)
        {
            if (dashFX != null)
            {
                audioSource.audio = dashFX;
            }
            if (!dashPlayed)
            {
                audioSource.PlayAudio();
                dashPlayed = true;
            }
        }
        else
        {
            Debug.Log("ALITA'S AUDIO SOURCE MISSING");
        }
    }

    public void PlayDiyingFX(AudioSource audioSource)
    {
        if (audioSource != null)
        {
            PlayOnce(audioSource, dieSound);
            PlayOnce(audioSource, dieScream);
            Ambient.Instance.dead = true;
        }
        else
        {
            Debug.Log("AUDIOSOURCE MISSING IN ALITA");
        }
    }

    public void PlayHittedActor(AudioSource audioSource,Unit.Unit_Type unit_Type, float currentLife,float maxLife, string sound_1, string sound_2, string sound_3, float criticalHealthPercent = 0)
    {
        if (audioSource != null)
        {
            PlayRandomeSound(audioSource, sound_1, sound_2, sound_3);

            Debug.Log(criticalHealthPercent.ToString());
            Debug.Log(currentLife.ToString());

            if (currentLife < (criticalHealthPercent * maxLife/100) && !Ambient.Instance.diying && !Ambient.Instance.lowlife)
            {
                Ambient.Instance.lowlife = true;
            }
        }
        else
        {
            Debug.Log("AUDIOSOURCE MISSING IN ALITA");
        }
    }
}


