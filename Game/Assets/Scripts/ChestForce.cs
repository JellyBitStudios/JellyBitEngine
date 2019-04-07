using System.Collections;
using System;
using JellyBitEngine;

public class ChestForce : JellyScript
{
    #region PUBLIC_VARIABLES
    public float force = 10.0f;
    public GameObject Alita = null;
    #endregion

    public override void Start()
    {
        /*
        Random rand = new Random();
        byte[] bytes = new byte[3];
        rand.NextBytes(bytes);
                
        Vector3 direction = new Vector3(bytes[0], bytes[1], bytes[2]);
        */

        Vector3 direction = (transform.position - Alita.transform.position).normalized();
        gameObject.GetComponent<Rigidbody>().AddForce(direction * force, Rigidbody.ForceMode.eIMPULSE);
    }
}