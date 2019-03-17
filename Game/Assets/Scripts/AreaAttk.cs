using System.Collections;
using JellyBitEngine;

public class AreaAttk : JellyScript
{
    public GameObject areaCircle = null;
    private bool isAreaActive = false;

    //Area propeties
    public float circleRadius = 5.0f;
    public int areaDamage = 30;

    //Area cooldown
    private bool isAreaCooldown = false;
    float timeCoolDown = 3.0f;
    float cooling = 0.0f;

    public override void Awake()
    {

    }

    public override void Update()
    {
        if (isAreaCooldown)
            CoolingDown();
    }

    public void ToggleAreaVisibility()
    {  
        if (!isAreaActive)
        {
            if (areaCircle != null)
                areaCircle.active = true;
            isAreaActive = true;
            Debug.Log("ACIVATE AREA");
        }
        
        else if (isAreaActive)
        {
            if (areaCircle != null)
                areaCircle.active = false;
            isAreaActive = false;
            Debug.Log("DESACTIVATE AREA");
        }

    }

    public bool HideArea()
    {
        bool activeArea = isAreaActive;

        if (isAreaActive)
        {
            if (areaCircle != null)
                areaCircle.active = false;
            isAreaActive = false;
            Debug.Log("DESACTIVATE AREA");
        }

        return activeArea;
    }

    public bool IsAreaActive()
    {
        return isAreaActive;
    }

    public void AreaAttack(LayerMask enemyMask)
    {
        Debug.Log("AREA ATTACK!!!!!");

        OverlapHit[] hitInfo;
        if (Physics.OverlapSphere(circleRadius, transform.position, out hitInfo, enemyMask, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
        {
            foreach (OverlapHit hit in hitInfo)
            {
                hit.gameObject.GetComponent<Unit>().Hit(areaDamage); //Not change this
                Debug.Log("HIT ENEMY: " + hit.gameObject.name);
            }
        }

        isAreaCooldown = true;
    }

    private void CoolingDown()
    {
        cooling += Time.deltaTime;
        if(cooling >= timeCoolDown)
        {
            Debug.Log("AreaCool");
            isAreaCooldown = false;
            cooling = 0.0f;
        }
    }


    public bool IsAreaCooldown()
    {
        return isAreaCooldown;
    }

}
