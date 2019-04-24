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
    public string Q_material = "";

    public Vector3 W_pos;
    public float W_FOV;
    public string W_material = "";


    AreaHab currHab = AreaHab.None;
    Projector projector;
    bool isActive = false;
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
                SetDecall(Q_pos, Q_FOV, Q_material, AreaHab.Q_area);

            if (!isActive)
            {
                projector.SetActive(true);
                isActive = true;
            }
        }
        else if (Input.GetKeyDown(KeyCode.KEY_W))
        {
            if (currHab != AreaHab.W_area)
                SetDecall(W_pos, W_FOV, W_material, AreaHab.W_area);

            Vector3 direction = (Player.lastRaycastHit.point - transform.position).normalized();
            transform.rotation = Quaternion.LookAt(Vector3.forward, direction, Vector3.up, transform.up);

            if (!isActive)
            {
                projector.SetActive(true);
                isActive = true;
            }
        }

        if (Input.GetKeyUp(KeyCode.KEY_Q) || Input.GetKeyUp(KeyCode.KEY_W))
        {
            projector.SetActive(false);
            isActive = false;
        }
    }

    private void SetDecall(Vector3 pos, float fov, string mat, AreaHab type)
    {
        transform.localPosition = pos;
        projector.FOV = fov;
        projector.SetResource(mat);
        currHab = type;
    }
}

