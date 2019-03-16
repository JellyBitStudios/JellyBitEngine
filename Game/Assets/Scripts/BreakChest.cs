using System.Collections;
using JellyBitEngine;
using System;

public class BreakChest : JellyScript
{

    public GameObject brokeChest = null;
    public LayerMask chest_layer = new LayerMask();

    public override void Update()
    {
        if (Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            RaycastHit hit;
            if (Physics.Raycast(ray, out hit, float.MaxValue, chest_layer, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                Debug.Log("Raycast collision detected");
                GameObject newgo = GameObject.Instantiate(brokeChest, hit.point);
                gameObject.active = false;
            }
        }

    }

}