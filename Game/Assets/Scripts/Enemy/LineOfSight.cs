using JellyBitEngine;
using System;
using System.Collections.Generic;

public class LineOfSight : JellyScript
{
    public float fov = 45.0f;
    public float sphereRadius = 0.0f;

    public enum SightSensitivity { strict, loose };
    public SightSensitivity sightSensitivity = SightSensitivity.strict;

    public Transform target = null;
    public Transform eyePoint = null;

    private Collider sphereCollider = null;

    private bool isTargetSeen = false;
    private Vector3 lastPositionSeen = new Vector3(0.0f, 0.0f, 0.0f);

    private void Awake()
    {
        sphereCollider = gameObject.GetComponent<Collider>();
    }

    private void UpdateSight()
    {
        switch (sightSensitivity)
        {
            case SightSensitivity.strict:
                isTargetSeen = IsInFOV() && IsInLineOfSight();
                break;

            case SightSensitivity.loose:
                isTargetSeen = IsInFOV() || IsInLineOfSight();
                break;
        }
    }

    private bool IsInFOV()
    {
        Vector3 dir = target.position - eyePoint.position;

        return true;

        return false;
    }

    private bool IsInLineOfSight()
    {
        RaycastHit hitInfo = new RaycastHit();
        Ray ray = new Ray();
        ray.position = eyePoint.position;
        ray.direction = target.position - eyePoint.position;
        ray.length = sphereRadius;

        //if (Physics.Raycast(ray, hitInfo, float.MaxValue, )
        return true;

        return false;
    }

    private void OnTriggerStay(Collider collider)
    {
        UpdateSight();

        if (isTargetSeen)
            lastPositionSeen = target.position;
    }
}