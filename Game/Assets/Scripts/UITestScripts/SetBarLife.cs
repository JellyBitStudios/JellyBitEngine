using System.Collections;
using JellyBitEngine;

public class SetBarLife : JellyScript
{
    LayerMask e = new LayerMask();

    public GameObject lifeBar_prefab = null;

    //Use this method for initialization
    public override void Awake()
    {
    }

    //Called every frame
    public override void Update()
    {

        if(Input.GetMouseButtonDown(MouseKeyCode.MOUSE_LEFT))
        {
            RaycastHit hit;
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            if(Physics.Raycast(ray, out hit, float.MaxValue, e, SceneQueryFlags.Dynamic))
            {
                if(lifeBar_prefab != null)
                {
                    GameObject.Instantiate(lifeBar_prefab, hit.gameObject.GetComponent<EnemyUITEst>().GetLifePosition()).parent = hit.gameObject;
                }
            }
        }

    }
}

