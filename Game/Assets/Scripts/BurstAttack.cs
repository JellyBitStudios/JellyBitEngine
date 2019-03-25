using System.Collections;
using JellyBitEngine;

public class BurstAttack : JellyScript
{
    public GameObject burst;
    public GameObject instantiatePos;

    //Cool down
    private bool isBurstCoolDown = false;
    public float timeCoolDown = 3.0f;
    public float cooling = 0.0f;

    public override void Update()
    {
        if (isBurstCoolDown)
            CoolDown();
    }

    public void StartBurstAttack()
    {
        if (burst != null && instantiatePos != null) 
            GameObject.Instantiate(burst, instantiatePos.transform.position);

        isBurstCoolDown = true;
    }

    private void CoolDown()
    {
        cooling += Time.deltaTime;
        if (cooling >= timeCoolDown)
        {
            Debug.Log("BurstCool");
            isBurstCoolDown = false;
            cooling = 0.0f;
        }
    }

    public bool IsBurstCoolDown()
    {
        return isBurstCoolDown;
    }
}

