using System.Collections;
using JellyBitEngine;
using JellyBitEngine.SceneManager;

public class SceneChanger : JellyScript
{
    public string SceneToLoad = "Scene2";

    //Use this method for initialization
    public override void Awake()
    {

    }

    //Called every frame
    public override void Update()
    {
        if(Input.GetKeyDown(KeyCode.KEY_1))
        {
            SceneManager.LoadScene(SceneToLoad);
        }
    }
}

