using System.Collections;
using System;
using JellyBitEngine;



public class ChestForce : JellyScript
{

    public override void Awake()
    {
        Random rand = new Random();
        byte[] bytes = new byte[3];
        rand.NextBytes(bytes);

        Vector3 direction = new Vector3 (bytes[0], bytes[1], bytes[2]);
        gameObject.GetComponent<Rigidbody>().AddForce(direction.normalized() *10, Rigidbody.ForceMode.eIMPULSE);
    } 

}