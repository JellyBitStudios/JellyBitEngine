using System.Collections;
using JellyBitEngine;

public class AlitaAudio : JellyScript
{
    //Audio Source
    private AudioSource audioSource = null;
    //Alita states
    private enum Alita_State
    {
        IDLE,
        RUNNING,
        GOING_TO_ATTK,
        ATTK,
        DASHING,
        AREA_ATTK
    }
    Alita_State lastState = Alita_State.IDLE;

    public void SetLastStateAttack() { lastState = Alita_State.ATTK; }


    public override void Awake()
    {
        audioSource = gameObject.GetComponent<AudioSource>();
    }

    public void PlayIdle()
    {
        ModuleAudio.Instance.PlayIdleFX(audioSource);
    }

    public void CheckToPlayIdle()
    {
        if (audioSource != null)
        {
            if (lastState == Alita_State.RUNNING)
            {
                ModuleAudio.Instance.CleanAudio(audioSource);//solo si vienes de correr o dashear
                Debug.Log("LO ULTIMO KE EE ESHO A SIO CORRER");
            }
            //if (lastState == Alita_State.DASHING)
            //{
            //    ModuleAudio.Instance.CleanAudio(audioSource);//solo si vienes de correr o dashear
            //    Debug.Log("LO ULTIMO KE EE ESHO A SIO DASHEAR");
            //}
            ModuleAudio.Instance.CanPlayRunning();
        }
        lastState = Alita_State.IDLE;
    }

    public void PlayRunning()
    {
        if (audioSource != null && ModuleAudio.Instance.canPlayRunning)
        {
            ModuleAudio.Instance.ResetIdleCounter();
            ModuleAudio.Instance.CleanAudio(audioSource);
            ModuleAudio.Instance.PlayRunningFX(audioSource);
            ModuleAudio.Instance.canPlayRunning = false;
        }
        lastState = Alita_State.RUNNING;
    }

    public void PlayRunningGoToEnem()
    {
        if (audioSource != null)
        {
            ModuleAudio.Instance.ResetIdleCounter();
            ModuleAudio.Instance.CleanAudio(audioSource);
            ModuleAudio.Instance.CanPlayRunning();
            ModuleAudio.Instance.PlayRunningFX(audioSource);
        }
        lastState = Alita_State.GOING_TO_ATTK;
    }

    public void PlayHit()
    {
        if (audioSource != null)
        {
            ModuleAudio.Instance.ResetIdleCounter();
            ModuleAudio.Instance.CanPlayRunning();
            ModuleAudio.Instance.CleanAudio(audioSource);
            ModuleAudio.Instance.PlayRandomeSound(audioSource, ModuleAudio.Instance.hitScream_1,
                                                              ModuleAudio.Instance.hitScream_2,
                                                              ModuleAudio.Instance.hitScream_3);
        }
    }

    public void PlayDash()
    {
        if (audioSource != null)
        {
            if (lastState != Alita_State.DASHING)
            {
                ModuleAudio.Instance.CleanAudio(audioSource);
            }
            ModuleAudio.Instance.ResetIdleCounter();
            ModuleAudio.Instance.CanPlayRunning();
            ModuleAudio.Instance.PlayDashFX(audioSource);
        }

        lastState = Alita_State.DASHING;
    }


    public void PlayAreaAttackHit()
    {
        //Audio
        if (audioSource != null)
        {
            ModuleAudio.Instance.PlayOnce(audioSource, ModuleAudio.Instance.areaAttackFX);
            ModuleAudio.Instance.PlayOnce(audioSource, ModuleAudio.Instance.areaAttackScreamFX);
        }
        lastState = Alita_State.AREA_ATTK;
    }

    public void CleanAlitaAudio()
    {
        if (audioSource != null)
        {
            ModuleAudio.Instance.ResetIdleCounter();
            ModuleAudio.Instance.CanPlayRunning();
            ModuleAudio.Instance.CleanAudio(audioSource);
        }
    }

}