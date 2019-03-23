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
        Debug.Log("My gameObject layer's name is " + gameObject.GetLayer());

        foreach (GameObject child in gameObject.childs)
        {
            RaycastHit hit;
            Debug.Log("My child " + child.name + " is very pretty");
        }
    }
}

