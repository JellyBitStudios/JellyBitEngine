using System.Collections;
using System;
using JellyBitEngine;

public class BloodDecal : JellyScript
{
    #region PUBLIC_VARIABLES
    public GameObject Alita = null;
    public float distance = 1.0f;

    public GameObject reference = null;
    #endregion

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
            OrientDecal(Alita.transform.position, reference.transform.position, distance);
            ShowDecal();
        }
        else if (Input.GetKeyDown(KeyCode.KEY_S))
            HideDecal();
    }

    private void OrientDecal(Vector3 Alita, Vector3 position, float distance)
    {
        Debug.Log("Decal reference position: " + position);
        Vector3 direction = (position - Alita).normalized();
        Vector3 newPosition = position + direction * distance;
        Debug.Log("Decal reference new position: " + newPosition);

        transform.rotation = LookAt(newPosition);
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
        Vector3 direction = (position - transform.position).normalized();
        return Quaternion.LookAt(Vector3.forward, direction, Vector3.up, transform.up);
    }
}