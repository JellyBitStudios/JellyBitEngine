using System.Collections;
using System;
using JellyBitEngine;

public class BreakChest : JellyScript
{
    #region PUBLIC_VARIABLES
    public float height = 5.0f;
    public LayerMask layer = new LayerMask();
    public GameObject brokenChest = null;
    #endregion

    public override void Update()
    {
        if (Input.GetMouseButton(MouseKeyCode.MOUSE_LEFT))
        {
            Ray ray = Physics.ScreenToRay(Input.GetMousePosition(), Camera.main);
            RaycastHit hitInfo;
            if (Physics.Raycast(ray, out hitInfo, float.MaxValue, layer, SceneQueryFlags.Dynamic | SceneQueryFlags.Static))
            {
                Debug.Log("Chest ray hit!");

                Destroy(gameObject);
                Vector3 newPosition = new Vector3(transform.position.x, transform.position.y + height, transform.position.z);
                GameObject.Instantiate(brokenChest, newPosition);
            }
        }
    }
}