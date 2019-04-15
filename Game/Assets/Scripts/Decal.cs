﻿using System.Collections;
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
    #endregion

    public override void Awake()
    {
        projector = gameObject.GetComponent<Projector>();
    }

    public override void Start()
    {
        OrientDecal();
        ShowDecal(true);
    }

    public override void Update()
    {
        timer += Time.deltaTime;

        if (timer >= seconds)
            Destroy(gameObject);
    }

    private void OrientDecal()
    {
        Vector3 newPosition = transform.position;

        switch (decalType)
        {
            case DecalType.blood:

                Random random = new Random();
                int randomNumber = random.Next(minDistance, maxDistance);
                Debug.Log("Random number to orient the blood decal: " + randomNumber);

                Vector3 direction = (reference.transform.position - Alita.transform.position).normalized();
                newPosition = reference.transform.position + direction * randomNumber;

                break;

            case DecalType.brokenFloor:

                newPosition = reference.transform.position;

                break;
        }

        transform.rotation = LookAt(newPosition);
    }

    private void ShowDecal(bool show)
    {
        projector.SetActive(show);
    }

    private Quaternion LookAt(Vector3 position)
    {
        Vector3 direction = (position - transform.position).normalized();
        return Quaternion.LookAt(Vector3.forward, direction, Vector3.up, transform.up);
    }
}