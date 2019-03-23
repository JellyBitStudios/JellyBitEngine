using System.Collections;
using JellyBitEngine;

public class Test : JellyScript
{
    public string patato = "patata";

    public GameObject testGO;
    //Use this method for initialization

    public override void Awake()
    {
    }

    //Called every frame
    public override void Update()
    {
        Debug.ClearConsole();

        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            Material mat = gameObject.GetComponent<Material>();
            if (mat != null)
            {
                mat.SetResource("Blue");
            }
        }
        if (Input.GetKeyDown(KeyCode.KEY_2))
        {
            Material mat = gameObject.GetComponent<Material>();
            if (mat != null)
            {
                mat.SetResource("Default material");
            }
        }   
    }
}

