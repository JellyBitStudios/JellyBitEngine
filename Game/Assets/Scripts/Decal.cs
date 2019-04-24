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

    public float seconds = 10.0f;
    public int minDistance = 0;
    public int maxDistance = 3;
    #endregion

    #region PRIVATE_VARIABLES
    private Projector projector = null;
    private float timer = 0.0f;
    private bool isActive = false;
    #endregion

    public override void Awake()
    {
        projector = gameObject.GetComponent<Projector>();
    }

    public override void Update()
    {
        if (isActive)
        {
            timer += Time.deltaTime;

            if (timer >= seconds)
                Destroy(gameObject);
        }
    }

    public void OrientBloodDecal()
    {
        Vector3 newPosition = transform.position;

        Random random = new Random();
        int randomNumber = random.Next(minDistance, maxDistance);
        Debug.Log("Random number to orient the blood decal: " + randomNumber);

        Vector3 direction = (reference.transform.position - Alita.transform.position).normalized();
        newPosition = reference.transform.position + direction * randomNumber;

        transform.rotation = LookAt(newPosition);
    }

    public void OrientBrokenFloorDecal()
    {
        Vector3 newPosition = transform.position;

        newPosition = reference.transform.position;

        transform.rotation = LookAt(newPosition);
    }

    public void ShowDecal(bool show)
    {
        projector.SetActive(show);
        isActive = show;
    }

    private Quaternion LookAt(Vector3 position)
    {
        Vector3 direction = (position - transform.position).normalized();
        return Quaternion.LookAt(Vector3.forward, direction, Vector3.up, transform.up);
    }
}