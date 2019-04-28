using System.Collections;
using JellyBitEngine;

public class DropItemUITest : JellyScript
{
    public GameObject ItemDroped = null;

    public LayerMask terrain = new LayerMask();

    //Use this method for initialization
    public override void Awake()
    {
    }

    //Called every frame
    public override void Update()
    {
        if (Input.GetMouseButtonDown(MouseKeyCode.MOUSE_LEFT))
        {
            RaycastHit hit;
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            if (Physics.Raycast(ray, out hit, float.MaxValue, terrain, SceneQueryFlags.Static))
            {
                if (ItemDroped != null)
                {
                    GameObject.Instantiate(ItemDroped, hit.point).parent = hit.gameObject;
                }
            }
        }
    }
}

