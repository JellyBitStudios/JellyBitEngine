using System.Collections;
using System;
using JellyBitEngine;

public class Decal : JellyScript
{
    #region PUBLIC_VARIABLES
    public enum DecalType { blood, brokenFloor };
    public DecalType decalType = DecalType.blood;

    public GameObject reference = null;

    public GameObject Alita = null;
    public float distance = 1.0f;
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
            OrientDecal();
            ShowDecal();
        }
        else if (Input.GetKeyDown(KeyCode.KEY_S))
            HideDecal();
    }

    private void OrientDecal()
    {
        Vector3 newPosition = transform.position;

        switch (decalType)
        {
            case DecalType.blood:

                Vector3 direction = (reference.transform.position - Alita.transform.position).normalized();
                newPosition = reference.transform.position + direction * distance;

                break;

            case DecalType.brokenFloor:

                newPosition = reference.transform.position;

                break;
        }

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