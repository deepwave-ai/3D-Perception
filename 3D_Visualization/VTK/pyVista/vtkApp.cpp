// vtkApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkCubeSource.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkStreamTracer.h>
#include <vtkRungeKutta4.h>
#include <vtkArrowSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkDataSetMapper.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkLineSource.h>
#include <vtkProperty.h>

// Function to display a VTK actor
void DisplayActor(vtkSmartPointer<vtkActor> actor) {
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();

    renderWindow->AddRenderer(renderer);
    renderWindowInteractor->SetRenderWindow(renderWindow);

    renderer->AddActor(actor);
    renderer->SetBackground(0.1, 0.2, 0.3); // Background color

    renderWindow->Render();
    renderWindowInteractor->Start();
}

int main() {
    // 1. Sphere
    auto sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetRadius(1.0);
    sphereSource->SetCenter(0, 0, 0);
    sphereSource->Update();

    auto sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

    auto sphereActor = vtkSmartPointer<vtkActor>::New();
    sphereActor->SetMapper(sphereMapper);
    sphereActor->GetProperty()->SetColor(1.0, 0.0, 0.0); // Red

    DisplayActor(sphereActor);

    // 2. Cylinder
    auto cylinderSource = vtkSmartPointer<vtkCylinderSource>::New();
    cylinderSource->SetRadius(1.0);
    cylinderSource->SetHeight(2.0);
    cylinderSource->SetResolution(50);
    cylinderSource->Update();

    auto cylinderMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    cylinderMapper->SetInputConnection(cylinderSource->GetOutputPort());

    auto cylinderActor = vtkSmartPointer<vtkActor>::New();
    cylinderActor->SetMapper(cylinderMapper);
    cylinderActor->GetProperty()->SetColor(0.0, 0.0, 1.0); // Blue

    DisplayActor(cylinderActor);

    // 3. Boolean Operations: Sphere - Cube
    auto cubeSource = vtkSmartPointer<vtkCubeSource>::New();
    cubeSource->SetCenter(0.5, 0.5, 0.0);
    cubeSource->SetXLength(1.5);
    cubeSource->SetYLength(1.5);
    cubeSource->SetZLength(1.5);
    cubeSource->Update();

    auto booleanOperation = vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
    booleanOperation->SetOperationToDifference();
    booleanOperation->SetInputConnection(0, sphereSource->GetOutputPort());
    booleanOperation->SetInputConnection(1, cubeSource->GetOutputPort());
    booleanOperation->Update();

    auto booleanMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    booleanMapper->SetInputConnection(booleanOperation->GetOutputPort());

    auto booleanActor = vtkSmartPointer<vtkActor>::New();
    booleanActor->SetMapper(booleanMapper);
    booleanActor->GetProperty()->SetColor(1.0, 0.5, 0.0); // Orange

    DisplayActor(booleanActor);

    // 4. Structured Grid with Scalars
    int nx = 20, ny = 20, nz = 20;
    auto points = vtkSmartPointer<vtkPoints>::New();
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            for (int k = 0; k < nz; ++k) {
                double x = static_cast<double>(i) / nx * 2.0 - 1.0;
                double y = static_cast<double>(j) / ny * 2.0 - 1.0;
                double z = static_cast<double>(k) / nz * 2.0 - 1.0;
                points->InsertNextPoint(x, y, z);
            }
        }
    }

    auto structuredGrid = vtkSmartPointer<vtkStructuredGrid>::New();
    structuredGrid->SetDimensions(nx, ny, nz);
    structuredGrid->SetPoints(points);

    auto scalars = vtkSmartPointer<vtkDoubleArray>::New();
    scalars->SetName("Scalars");
    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i) {
        double p[3];
        points->GetPoint(i, p);
        scalars->InsertNextValue(sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]));
    }
    structuredGrid->GetPointData()->SetScalars(scalars);

    auto gridMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    gridMapper->SetInputData(structuredGrid);

    auto gridActor = vtkSmartPointer<vtkActor>::New();
    gridActor->SetMapper(gridMapper);

    DisplayActor(gridActor);

    return 0;
}
