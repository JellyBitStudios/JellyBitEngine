using System.Collections;
using System;
using JellyBitEngine;

public class BloodDecal : JellyScript
{
    public GameObject reference = null;
    public float distance = 1.0f;

    #region PRIVATE_VARIABLES
    private Projector projector = null;
    #endregion

    public override void Awake()
    {
        projector = gameObject.GetComponent<Projector>();
    }

    public override void Update()
    {
        if (Input.GetKeyDown(KeyCode.KEY_A))
        {    
            OrientDecal(reference.transform.position);
            ShowDecal();
        }
        else if (Input.GetKeyDown(KeyCode.KEY_S))
            HideDecal();
    }

    private void OrientDecal(Vector3 direction)
    {
        transform.rotation *= LookAt(direction);
    }

    private void ShowDecal()
    {       
        projector.SetActive(true);
    }

    private void HideDecal()
    {
        projector.SetActive(false);
    }

    private Quaternion LookAt(Vector3 position)
    {
        Debug.Log("Reference position: " + position);
        Debug.Log("Position: " + transform.position);

        Vector3 Z = (position - transform.position);

        //Vector3 X = Cross(Vector3.up, Z).normalized();

        float angleZ = (float)(Math.Atan2(Z.y, Z.x) - Math.Atan2(transform.forward.y, transform.forward.x));
        Quaternion rotationZ = Quaternion.Rotate(transform.right, angleZ);

        //float angleX = (float)Math.Atan2(transform.right.magnitude, X.magnitude);
        //Quaternion rotationX = Quaternion.Rotate(transform.up, angleX);

        return /*rotationX **/ rotationZ;
    }

    public static Vector3 Cross(Vector3 lhs, Vector3 rhs)
    {
        return new Vector3(
            lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.x * rhs.y - lhs.y * rhs.x);
    }
}