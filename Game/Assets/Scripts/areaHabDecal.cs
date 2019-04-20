using System.Collections;
using JellyBitEngine;

public class areaHabDecal : JellyScript
{
    public enum AreaHab
    {
        None,
        Q_area,
        W_area
    };

    public Vector3 Q_pos;
    public float Q_FOV;
    //public string Q_material;

    public Vector3 W_pos;
    public float W_FOV;
    //public string W_material;


    AreaHab currHab = AreaHab.None;
    Projector projector;
    //Use this method for initialization
    public override void Awake()
    {
        projector = gameObject.GetComponent<Projector>();
    }

    //Called every frame
    public override void Update()
    {
        if (Input.GetKeyDown(KeyCode.KEY_Q))
        {
            if (currHab != AreaHab.Q_area)
            {
                transform.localPosition = Q_pos;
                projector.FOV = Q_FOV;
                projector.SetResource("Q_areaDecal");
                currHab = AreaHab.Q_area;
            }
            projector.SetActive(true);
        }
        else if (Input.GetKeyDown(KeyCode.KEY_W))
        {
            if (currHab != AreaHab.W_area)
            {
                transform.localPosition = W_pos;
                projector.FOV = W_FOV;
                projector.SetResource("W_areaDecal");

                currHab = AreaHab.W_area;
            }
            projector.SetActive(true);
        }

        if (Input.GetKeyUp(KeyCode.KEY_Q) || Input.GetKeyUp(KeyCode.KEY_W))
            projector.SetActive(false);
    }
}

